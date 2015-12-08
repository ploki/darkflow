#ifndef WORKERLOADVIDEO_H
#define WORKERLOADVIDEO_H

#include <QObject>
#include <QVector>
#include <QString>
#include "operatorworker.h"

struct AVFrame;
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
    void play();
    bool decodeVideo(const QString& filename, int progress, int complete);
    void push_frame(AVFrame *picture,
                    const QString &filename, int progress, int complete, int n, int c);
private:
    QVector<QString> m_collection;
};

#endif // WORKERLOADVIDEO_H
