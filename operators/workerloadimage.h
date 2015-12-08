#ifndef WORKERLOADIMAGE_H
#define WORKERLOADIMAGE_H

#include <QObject>

#include "operatorworker.h"
#include "oploadimage.h"

class WorkerLoadImage : public OperatorWorker
{
    Q_OBJECT
public:
    WorkerLoadImage(QVector<QString> filesCollection,
                    OpLoadImage::ColorSpace colorSpace,
                    QThread *thread,
                    Operator *op);
    Photo process(const Photo &, int, int);
    void play();
private:
    QVector<QString> m_filesCollection;
    OpLoadImage::ColorSpace m_colorSpace;
};

#endif // WORKERLOADIMAGE_H
