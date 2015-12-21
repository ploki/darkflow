#include <QObject>
#include <QFileInfo>
#include <QStringList>

#include <Magick++.h>

#include "workerloadvideo.h"
#include "oploadvideo.h"
#include "algorithm.h"
#include "operatorparameterslider.h"
#include "console.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}


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
static bool handle_error(const char *str) {
    dflWarning("LoadVideo(Worker): %s", str);
    return false;
}

bool WorkerLoadVideo::decodeVideo(const QString &filename, int progress, int complete)
{
    av_register_all();

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx, filename.toLocal8Bit(), NULL, NULL)!=0)
            return handle_error("Couldn't open file");

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
            return handle_error("Couldn't find stream information");
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
        return handle_error("Didn't find a video stream");

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;


    AVCodec *pCodec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
        return handle_error("Codec not found");

    // Inform the codec that we can handle truncated bitstreams -- i.e.,
    // bitstreams where frame boundaries can fall in the middle of packets
    if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
        pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

    if ( pCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P &&
         pCodecCtx->pix_fmt != AV_PIX_FMT_YUV410P)
            return handle_error("pix_fmt != PIX_FMT_YUV420P");

    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
        return handle_error("Could not open codec");

    AVFrame *picture;
    int fps_num = pCodecCtx->framerate.num;
    int fps_den = pCodecCtx->framerate.den;
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
                 dflDebug("Error while decoding frame");
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
                 av_free_packet(&avpkt);
                 avpkt.data = NULL;
                 avpkt.size = 0;
             }
             if (av_read_frame(pFormatCtx, &avpkt))
                 goto loop_exit;
         } while(avpkt.stream_index != videoStream );
     }
loop_exit:
     len = avcodec_decode_video2(pCodecCtx, picture, &got_picture, &avpkt);
     if (got_picture) {
         fflush(stdout);

         /* the picture is allocated by the decoder. no need to
            free it */
         push_frame(picture, filename, progress, complete, frame, frame_count);
         frame++;
         if (avpkt.data)
             av_free_packet(&avpkt);
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
        default:
            dflWarning("LoadVideo(Worker): Unsupported pixel format");
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
            image.modifyImage();
            Magick::Pixels pixel_cache(image);
#pragma omp parallel for
            for ( int y = 0 ; y < h ; ++y ) {
                Magick::PixelPacket *pixels = pixel_cache.get(0, y, w, 1);
                if (!pixels) {
                    dflWarning("LoadVideo(Worker): NULL pixels!");
                    continue;
                }
                for ( int x = 0 ; x < w ; ++x ) {
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
                }
            }
            pixel_cache.sync();
            outputPush(0, photo);
            --m_count;
        }
        catch (std::exception &e) {
            dflError(e.what());
            m_error = true;
            return false;
        }
    }
    emitProgress(progress, complete, n%c, c);
    if ( m_count == 0 )
        return false;
    return true;
}


