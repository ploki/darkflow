#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QUrl>
#include "workerloadvideo.h"
#include "oploadvideo.h"
#include <Magick++.h>

class VideoSurface : public QAbstractVideoSurface {

public:
    VideoSurface(WorkerLoadVideo *w);
    ~VideoSurface();
public slots:
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
    bool present(const QVideoFrame &frame);

private:
    WorkerLoadVideo *m_w;

};
VideoSurface::VideoSurface(WorkerLoadVideo *w) :
    QAbstractVideoSurface(),
    m_w(w)
{
}

VideoSurface::~VideoSurface()
{

}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    // Return the formats you will support
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB24;
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    qDebug("One frame");
    int w,h;
    QVideoFrame frame2(frame);
    frame2.map(QAbstractVideoBuffer::ReadOnly);
    w = frame2.bytesPerLine()/3;
    h = frame2.height();
    if ( frame2.pixelFormat() != QVideoFrame::Format_RGB24) {
        qDebug(QString("Unexpected pixel format: %0").arg(frame.pixelFormat()).toLatin1());
        return true;
        m_w->emitFailure();
    }
    Photo photo(Photo::sRGB);
    photo.setIdentity(QString("%0[%1]").arg(m_w->m_currentFile).arg(m_w->m_currentFrame));
    photo.createImage(w, h);
    Magick::Image& image=photo.image();
    image.modifyImage();
    Magick::Pixels pixel_cache(image);
    const uchar *src = frame2.bits();
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0, y, w, 1);
        if (!pixels) {
            qWarning("NULL pixels!");
            continue;
        }
        for ( int x = 0 ; x < w ; ++x ) {
            pixels[x].red = src[y*w*3+x*3+0]<<8;
            pixels[x].green = src[y*w*3+x*3+1]<<8;
            pixels[x].blue = src[y*w*3+x*3+2]<<8;
        }
    }
    pixel_cache.sync();
    photo.setSequenceNumber(m_w->m_currentFrame);
    photo.setTag("Name",photo.getIdentity());
    m_w->m_outputs[0].push_back(photo);
    ++m_w->m_currentFrame;
    frame2.unmap();
    return true;
}




WorkerLoadVideo::WorkerLoadVideo(QThread *thread, OpLoadVideo *op) :
    OperatorWorker(thread, op),
    m_collection(op->getCollection().toVector()),
    m_currentFile(),
    m_currentFrame(0),
    m_i(0),
    m_player(new QMediaPlayer),
    m_surf(new VideoSurface(this))
{
    connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(stateChanged(QMediaPlayer::MediaStatus)));
}

WorkerLoadVideo::~WorkerLoadVideo()
{
    delete m_player;
    delete m_surf;
}

Photo WorkerLoadVideo::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLoadVideo::play(QVector<QVector<Photo> > inputs, int n_outputs)
{
    m_inputs = inputs;
    play_prepareOutputs(n_outputs);

    m_i = -1;
    stateChanged(QMediaPlayer::EndOfMedia);
    return;

}

void WorkerLoadVideo::stateChanged(QMediaPlayer::MediaStatus status)
{
    if ( status == QMediaPlayer::EndOfMedia ) {
        ++m_i;
        if ( m_i >= m_collection.count()) {
            qDebug("video emit success");
            emitSuccess();
            return;
        }
        m_currentFile = m_collection[m_i];
        m_currentFrame = 0;
        m_player->setMedia(QUrl("file://"+m_currentFile));
        m_player->setVideoOutput(m_surf);
        m_player->setMuted(true);
        //m_player->setPlaybackRate(10000.);
        qDebug(QString("player error: %0").arg(m_player->error()).toLatin1());
        qDebug(QString("media status: %0").arg(m_player->mediaStatus()).toLatin1());
        m_player->play();

        qDebug(QString("player error: %0").arg(m_player->error()).toLatin1());
        qDebug(QString("media status: %0").arg(m_player->mediaStatus()).toLatin1());
    }

}

