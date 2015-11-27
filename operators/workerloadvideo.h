#ifndef WORKERLOADVIDEO_H
#define WORKERLOADVIDEO_H

#include <QAbstractVideoSurface>
#include <QObject>
#include <QVector>
#include <QString>
#include <QMediaPlayer>
#include "operatorworker.h"

class OpLoadVideo;
class QMediaPlayer;
class VideoSurface;

class WorkerLoadVideo : public OperatorWorker
{
    Q_OBJECT
public:
    WorkerLoadVideo(QThread *thread, OpLoadVideo *op);
    ~WorkerLoadVideo();
    Photo process(const Photo &photo, int p, int c);

private slots:
    void play(QVector<QVector<Photo> > inputs, int n_outputs);
public slots:
    void stateChanged(QMediaPlayer::MediaStatus);

private:
    friend class VideoSurface;
    QVector<QString> m_collection;
    QString m_currentFile;
    int m_currentFrame;
    int m_i;
    QMediaPlayer *m_player;
    VideoSurface *m_surf;
};

#endif // WORKERLOADVIDEO_H
