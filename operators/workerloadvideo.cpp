#include <QObject>
#include <QUrl>
#include "workerloadvideo.h"
#include "oploadvideo.h"
#include <QFile>
#include <Magick++.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}

#include "algorithm.h"

WorkerLoadVideo::WorkerLoadVideo(QThread *thread, OpLoadVideo *op) :
    OperatorWorker(thread, op),
    m_collection(op->getCollection().toVector())
{
}

WorkerLoadVideo::~WorkerLoadVideo()
{
}

Photo WorkerLoadVideo::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLoadVideo::play(QVector<QVector<Photo> > inputs, int n_outputs)
{
    m_inputs = inputs;
    play_prepareOutputs(n_outputs);
    for ( int i = 0, s = m_collection.count() ;
          i < s ;
          ++i ) {
        bool res = decodeVideo(m_collection[i], i, s);
        if ( !res ) {
            emitFailure();
            return;
        }
    }
    emitSuccess();
    return;
}
static bool handle_error(const char *str) {
    qWarning(str);
    return false;
}

bool WorkerLoadVideo::decodeVideo(const QString &filename, int progress, int complete)
{
    av_register_all();

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx, filename.toLatin1(), NULL, NULL)!=0)
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

    if ( pCodecCtx->pix_fmt != PIX_FMT_YUV420P )
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
    int64_t frame_count = (pFormatCtx->duration==0?1:pFormatCtx->duration/AV_TIME_BASE) *  fps_den / fps_num;
    int frame, got_picture, len;
    picture=av_frame_alloc();

    //QFile file(filename);
    //file.open(QFile::ReadOnly);
    AVPacket avpkt;
    //av_init_packet(&avpkt);
    avpkt.data=NULL;
    avpkt.size = 0;

    frame = 0;
     for(;;) {
         while (avpkt.size > 0) {
             len = avcodec_decode_video2(pCodecCtx, picture, &got_picture, &avpkt);
             if (len < 0) {
                 qDebug("Error while decoding frame");
              }
             avpkt.size-=len;
             avpkt.data+=len;
             if (got_picture) {
                 push_frame(picture, filename, progress, complete, frame, frame_count);
                 frame++;
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
     return true;
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


void WorkerLoadVideo::push_frame(AVFrame *picture,
                                 const QString &filename, int progress, int complete, int n, int c)
{
    int w = picture->width;
    int h = picture->height;
    Q_ASSERT( picture->format == PIX_FMT_YUV420P );

    Photo photo(Photo::sRGB);
    photo.setIdentity(QString("%0[%1]").arg(filename).arg(n));
    photo.createImage(w, h);
    Magick::Image& image=photo.image();
    image.modifyImage();
    Magick::Pixels pixel_cache(image);
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0, y, w, 1);
        if (!pixels) {
            qWarning("NULL pixels!");
            continue;
        }
        for ( int x = 0 ; x < w ; ++x ) {
            Q_ASSERT( x/2 < picture->linesize[1] );
            Q_ASSERT( x/2 < picture->linesize[2] );
            const unsigned char c_y = picture->data[0][picture->linesize[0]*y + x];
            const unsigned char c_u = picture->data[1][picture->linesize[1]*(y/2) + x/2];
            const unsigned char c_v = picture->data[2][picture->linesize[2]*(y/2) + x/2];

            quantum_t r, g, b;
            yuv420_to_rgb(c_y,c_u,c_v,r,g,b);
            pixels[x].red = r;
            pixels[x].green = g;
            pixels[x].blue = b;
        }
    }
    pixel_cache.sync();
    photo.setSequenceNumber(n);
    photo.setTag("Name",photo.getIdentity());
    m_outputs[0].push_back(photo);
    emitProgress(progress, complete, n%c, c);
}


