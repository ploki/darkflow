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
    virtual Photo process(const Photo &photo, int p, int c) = 0;
    void finished();

signals:
    void progress(int ,int);
    void start();
    void success();
    void failure();

protected:
    QThread *m_thread;
    Operator *m_operator;
    bool m_signalEmited;

    bool aborted();
    void emitFailure();
    void emitSuccess();
    void emitProgress(int p, int c, int sub_p, int sub_c);
    bool play_inputsAvailable();
    bool play_outputsAvailable();
    virtual void play_analyseSources();
    virtual bool play_onInput(int idx);
};

#endif // OPERATORWORKER_H
