#ifndef OPERATORWORKER_H
#define OPERATORWORKER_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

#include "ports.h"
#include "photo.h"
#include "operator.h"

class QThread;


class OperatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit OperatorWorker(QThread *thread, Operator* op);

    void start(QVector<QVector<Photo> > inputs, QVector<Operator::OperatorOutputStatus> outputStatus);

    int outputsCount();
    void outputPush(int idx, const Photo& photo);
    void outputSort(int idx);

    bool aborted();

    virtual void play();
protected slots:
    void started();
    virtual Photo process(const Photo &photo, int p, int c) = 0;
    void finished();

signals:
    void progress(int ,int);
    void doStart();
    void success(QVector<QVector<Photo> >);
    void failure();

protected:
    QThread *m_thread;
    Operator *m_operator;
    QVector<QVector<Photo> > m_inputs;
private:
    QVector<QVector<Photo> > m_outputs;
    QVector<Operator::OperatorOutputStatus> m_outputStatus;
    QElapsedTimer m_elapsed;
protected:
    bool m_signalEmited;
    mutable bool m_error;
    bool m_earlyAbort;

    void emitFailure();
    void emitSuccess();
    void emitProgress(int p, int c, int sub_p, int sub_c);
    bool play_inputsAvailable();
    bool play_outputsAvailable();
    virtual void play_analyseSources();
    virtual bool play_onInput(int idx);
    virtual bool play_onInputParallel(int idx);

public:
    void dflDebug(const char* fmt, ...) const DF_PRINTF_FORMAT(2,3);
    void dflInfo(const char* fmt, ...) const DF_PRINTF_FORMAT(2,3);
    void dflWarning(const char* fmt, ...) const DF_PRINTF_FORMAT(2,3);
    void dflError(const char* fmt, ...) const DF_PRINTF_FORMAT(2,3);
    void dflCritical(const char* fmt, ...) const DF_PRINTF_FORMAT(2,3);
    void dflDebug(const QString& msg) const;
    void dflInfo(const QString& msg) const;
    void dflWarning(const QString& msg) const;
    void dflError(const QString& msg) const;
    void dflCritical(const QString& msg) const;

    void setError(const Photo& photo, const QString &msg) const;

private:
    void prepareOutputs(QVector<Operator::OperatorOutputStatus> outputStatus);
};

#endif // OPERATORWORKER_H
