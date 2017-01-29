/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include <QObject>
#include <QFileInfo>
#include <QStringList>

#include <Magick++.h>

#include "workerloadvideo.h"
#include "oploadvideo.h"
#include "algorithm.h"
#include "operatorparameterslider.h"
#include "console.h"

#ifdef HAVE_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}
#endif

WorkerLoadVideo::WorkerLoadVideo(QThread *thread, OpLoadVideo *op) :
    OperatorWorker(thread, op),
    m_collection(op->getCollection().toVector()),
    m_skip(op->getSkip()),
    m_count(op->getCount()),
    m_error(false)
{
}

WorkerLoadVideo::~WorkerLoadVideo()
{
}

Photo WorkerLoadVideo::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLoadVideo::play()
{
    for ( int i = 0, s = m_collection.count() ;
          i < s ;
          ++i ) {
        int skip = m_skip;
        int count = m_count;
        bool res = decodeVideo(m_collection[i], i, s);
        m_skip = skip;
        m_count = count;
        if ( !res ) {
            emitFailure();
            return;
        }
    }
    emitSuccess();
    return;
}

#ifdef HAVE_FFMPEG

static bool handle_error(const QString &str) {
    dflWarning(WorkerLoadVideo::tr("LoadVideo(Worker): %0").arg(str));
    return false;
}

bool WorkerLoadVideo::decodeVideo(const QString &filename, int progress, int complete)
{
    av_register_all();

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx, filename.toLocal8Bit(), NULL, NULL)!=0)
            return handle_error(tr("Couldn't open file %0").arg(filename));

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
            return handle_error(tr("Couldn't find stream information in file %0").arg(filename));
    unsigned int            i;
    int videoStream;
    AVCodecContext *pCodecCtx;
    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
            break;
        }
    if(videoStream==-1)
        return handle_error(tr("Didn't find a video stream for file %0").arg(filename));

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;


    AVCodec *pCodec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
        return handle_error(tr("Codec not found for file %0").arg(filename));

    // Inform the codec that we can handle truncated bitstreams -- i.e.,
    // bitstreams where frame boundaries can fall in the middle of packets
    if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
        pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

    if ( pCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P &&
         pCodecCtx->pix_fmt != AV_PIX_FMT_YUV410P &&
         pCodecCtx->pix_fmt != AV_PIX_FMT_BGR24 )
            return handle_error(tr("Unsupported pixel format for file %0").arg(filename));
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
        return handle_error(tr("Could not open codec for file %0").arg(filename));

    AVFrame *picture;

    AVRational fps =
#if LIBAVCODEC_VERSION_MICRO >= 100 && LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(56, 13, 100) // ffmpeg 2.5
    av_mul_q(pCodecCtx->framerate, av_make_q(pCodecCtx->ticks_per_frame, 1));
#else
    av_inv_q(pCodecCtx->time_base);
#endif

    int fps_num = fps.num;
    int fps_den = fps.den;
    if ( !fps_num || !fps_den ) {
        fps_num=1; fps_den=30;
    }
    if ( fps_num > fps_den ) {
        int tmp = fps_num;
        fps_num = fps_den;
        fps_den = tmp;
    }
    int64_t frame_count = (pFormatCtx->duration==0?AV_TIME_BASE:pFormatCtx->duration/AV_TIME_BASE) *  fps_num / fps_den;
    if ( !frame_count )
        frame_count=30;
    int frame, got_picture, len;
    picture=av_frame_alloc();

    AVPacket avpkt;
    //av_init_packet(&avpkt);
    avpkt.data=NULL;
    avpkt.size = 0;

    frame = 0;
     for(;;) {
         while (avpkt.size > 0) {
             len = avcodec_decode_video2(pCodecCtx, picture, &got_picture, &avpkt);
             if (len < 0) {
                 dflDebug(tr("Error while decoding frame in %0").arg(filename));
              }
             avpkt.size-=len;
             avpkt.data+=len;
             if (got_picture) {
                 bool cont = push_frame(picture, filename, progress, complete, frame, frame_count);
                 frame++;
                 if ( !cont ) goto loop_exit;
             }
         }
         do {
             if (avpkt.data) {
                 av_packet_unref(&avpkt);
                 avpkt.data = NULL;
                 avpkt.size = 0;
             }
             if (av_read_frame(pFormatCtx, &avpkt))
                 goto loop_exit;
         } while(avpkt.stream_index != videoStream );
     }
loop_exit:
     avcodec_decode_video2(pCodecCtx, picture, &got_picture, &avpkt);
     if (got_picture) {
         fflush(stdout);

         /* the picture is allocated by the decoder. no need to
            free it */
         push_frame(picture, filename, progress, complete, frame, frame_count);
         frame++;
         if (avpkt.data)
             av_packet_unref(&avpkt);
     }

     avcodec_close(pCodecCtx);
     av_free(pCodecCtx);
     av_free(picture);
     return !m_error;
}

static inline void yuv420_to_rgb(unsigned char y, unsigned char u, unsigned char v,
                          quantum_t& r, quantum_t& g, quantum_t& b) {
    int c = -16;
    int d = -128;
    int e = -128;
    c+=y;
    d+=u;
    e+=v;
    r = clamp( (298 * c + 409 * e + 128) );
    g = clamp( (298 * c - 100 * d - 208 * e + 128));
    b = clamp(298 * c + 516 * d + 128);
}

/**
 * @brief WorkerLoadVideo::push_frame
 * @param picture
 * @param filename
 * @param progress
 * @param complete
 * @param n
 * @param c
 * @return false if no more frame to grab because of m_count falls to 0
 */
bool WorkerLoadVideo::push_frame(AVFrame *picture,
                                 const QString &filename, int progress, int complete, int n, int c)
{
    if ( m_error )
        return false;

    if ( m_skip ) {
        --m_skip;
    }
    else if ( m_count ) {
        int w = picture->width;
        int h = picture->height;
        int div = 1;
        switch ( picture->format ) {
        case AV_PIX_FMT_YUV420P:
            div = 2; break;
        case AV_PIX_FMT_YUV410P:
            div = 4; break;
        case AV_PIX_FMT_BGR24:
            break;
        default:
            dflWarning(tr("LoadVideo(Worker): Unsupported pixel format in file %0").arg(filename));
            return false;
        }

        try {
            Photo photo(Photo::sRGB);
            photo.createImage(w, h);
            QFileInfo finfo(filename);
            QString name = QString("%0[%1]").arg(finfo.fileName()).arg(n);
            photo.setIdentity(m_operator->uuid() + "/" + name);
            photo.setTag(TAG_NAME,name);
            photo.setSequenceNumber(n);
            photo.setTag(TAG_SCALE, TAG_SCALE_NONLINEAR);
            Magick::Image& image=photo.image();
            std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
            dfl_block bool error = false;
            dfl_parallel_for(y, 0, h, 4, (image), {
                Magick::PixelPacket *pixels = pixel_cache->get(0, y, w, 1);
                if (error || !pixels) {
                    if (!error)
                        dflWarning(DF_NULL_PIXELS);
                    error = true;
                    continue;
                }
                for ( int x = 0 ; x < w ; ++x ) {
                    switch ( picture->format ) {
                    case AV_PIX_FMT_BGR24: {
                        const unsigned char c_b = picture->data[0][picture->linesize[0]*y + 3*x];
                        const unsigned char c_g = picture->data[0][picture->linesize[0]*y + 3*x+1];
                        const unsigned char c_r = picture->data[0][picture->linesize[0]*y + 3*x+2];

                        pixels[x].red = c_r*257;
                        pixels[x].green = c_g*257;
                        pixels[x].blue = c_b*257;
                        break;
                    }
                    default: {
                        Q_ASSERT( x/div < picture->linesize[1] );
                        Q_ASSERT( x/div < picture->linesize[2] );
                        const unsigned char c_y = picture->data[0][picture->linesize[0]*y + x];
                        const unsigned char c_u = picture->data[1][picture->linesize[1]*(y/div) + x/2];
                        const unsigned char c_v = picture->data[2][picture->linesize[2]*(y/div) + x/div];

                        quantum_t r, g, b;
                        yuv420_to_rgb(c_y,c_u,c_v,r,g,b);
                        pixels[x].red = r;
                        pixels[x].green = g;
                        pixels[x].blue = b;
                        break;
                    }
                    }
                }
                pixel_cache->sync();
            });
            outputPush(0, photo);
            --m_count;
        }
        catch (std::exception &e) {
            dflError("%s", e.what());
            m_error = true;
            return false;
        }
    }
    emitProgress(progress, complete, n%c, c);
    if ( m_count == 0 )
        return false;
    return true;
}
#else
bool WorkerLoadVideo::push_frame(AVFrame *,
                                 const QString &, int, int, int, int)
{
    dflError(tr("FFMPEG not compiled in"));
    return false;
}
bool WorkerLoadVideo::decodeVideo(const QString &, int, int)
{
    dflError(tr("FFMPEG not compiled in"));
    return false;
}
#endif
