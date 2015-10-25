#ifndef OPERATORWORKER_H
#define OPERATORWORKER_H

#include <QObject>

class QThread;
class Operator;
class Image;

class OperatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit OperatorWorker(QThread *thread, Operator* op);
private slots:
    virtual void play();
    virtual Image *process(const Image *image) = 0;

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
};

#endif // OPERATORWORKER_H
