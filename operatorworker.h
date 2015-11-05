#ifndef OPERATORWORKER_H
#define OPERATORWORKER_H

#include <QObject>

#include "photo.h"

class QThread;
class Operator;


class OperatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit OperatorWorker(QThread *thread, Operator* op);
private slots:
    virtual void play();
    virtual Photo process(const Photo *photo)
    { Q_UNUSED(photo); Q_ASSERT(!"Not Implemented");}

signals:
    void progress(int ,int);
    void start();
    void success();
    void failure();

protected:
    QThread *m_thread;
    Operator *m_operator;

    bool aborted();
    void emitFailure();
    void emitSuccess();
};

#endif // OPERATORWORKER_H
