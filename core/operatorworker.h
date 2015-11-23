#ifndef OPERATORWORKER_H
#define OPERATORWORKER_H

#include <QObject>
#include <QVector>

#include "photo.h"

class QThread;
class Operator;


class OperatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit OperatorWorker(QThread *thread, Operator* op);
protected slots:
    virtual void play(QVector<QVector<Photo> > inputs, int n_outputs);
    virtual Photo process(const Photo &photo, int p, int c) = 0;
    void finished();

signals:
    void progress(int ,int);
    void start(QVector<QVector<Photo> > inputs, int n_outputs);
    void success(QVector<QVector<Photo> >);
    void failure();

protected:
    QThread *m_thread;
    Operator *m_operator;
    QVector<QVector<Photo> > m_inputs;
    int m_n_outputs;
    QVector<QVector<Photo> > m_outputs;
    bool m_signalEmited;

    bool aborted();
    void emitFailure();
    void emitSuccess();
    void emitProgress(int p, int c, int sub_p, int sub_c);
    bool play_inputsAvailable();
    void play_prepareOutputs(int n_outputs);
    bool play_outputsAvailable();
    virtual void play_analyseSources();
    virtual bool play_onInput(int idx);

private:
    void play();
};

#endif // OPERATORWORKER_H
