#include <QThread>

#include "operatorworker.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"


OperatorWorker::OperatorWorker(QThread *thread, Operator *op) :
    QObject(NULL),
    m_thread(thread),
    m_operator(op),
    m_signalEmited(false)
{
    moveToThread(thread);
    connect(m_thread, SIGNAL(finished()), this, SLOT(deleteLater()));
    connect(this, SIGNAL(start()), this, SLOT(play()));
    connect(this, SIGNAL(progress(int,int)), m_operator, SLOT(workerProgress(int,int)));
    connect(this, SIGNAL(success()), m_operator, SLOT(workerSuccess()));
    connect(this, SIGNAL(failure()), m_operator, SLOT(workerFailure()));
    //m_thread->start();
}

void OperatorWorker::play()
{
    qWarning("OperatorWorker::play()");

    if ( !play_inputsAvailable() )
        return;
    if (!play_outputsAvailable())
        return;

    if ( play_parentDirty() )
        return;

    if ( play_isUpToDate() )
        return;

    play_analyseSources();

    play_onInput(0);
    Q_ASSERT(m_signalEmited);
}

bool OperatorWorker::aborted() {
    return m_thread->isInterruptionRequested();
}

void OperatorWorker::emitFailure() {
    m_signalEmited = true;
    emit progress(0, 1);
    emit failure();
}

void OperatorWorker::emitSuccess()
{
    m_signalEmited = true;
    emit progress(1, 1);
    emit success();
}

void OperatorWorker::emitProgress(int p, int c, int sub_p, int sub_c)
{
    emit progress( p * sub_c + sub_p , c * sub_c);
}

bool OperatorWorker::play_inputsAvailable()
{
    if ( 0 == m_operator->m_inputs.size() ) {
        qWarning("OperatorWorker::play() not overloaded for no input");
        emitFailure();
        return false;
    }
    return true;
}

bool OperatorWorker::play_outputsAvailable()
{
    if ( 0 == m_operator->m_outputs.size() ) {
        qWarning("OperatorWorker::play() not overloaded for #output != 1");
        emitFailure();
        return false;
    }
    return true;
}

bool OperatorWorker::play_parentDirty()
{
    bool dirty = false;
    foreach(OperatorInput *input, m_operator->m_inputs)
        foreach(OperatorOutput *parentOutput, input->sources()) {
            if ( !parentOutput->m_operator->isUpToDate() ) {
                dirty = true;
                parentOutput->m_operator->play();
                m_operator->m_waitingForParentUpToDate = true;
            }
        }
    if (dirty) {
        //will be signaled later
        emitFailure();
    }
    return dirty;
}

bool OperatorWorker::play_isUpToDate()
{
    if ( m_operator->isUpToDate())
    {
        emitSuccess();
        return true;
    }
    return false;
}

void OperatorWorker::play_analyseSources()
{

}

bool OperatorWorker::play_onInput(int idx)
{
    int c = 0;
    int p = 0;
    foreach(OperatorOutput *remoteOutput, m_operator->m_inputs[idx]->sources())
        c += remoteOutput->m_result.count();

    foreach(OperatorOutput *remoteOutput, m_operator->m_inputs[idx]->sources()) {
        const QVector<Photo> source = remoteOutput->m_result;
        foreach(const Photo &photo, source) {
            if ( aborted() ) {
                emitFailure();
                return false;
            }
            emit progress(p, c);
            Photo newResult = this->process(photo, p++, c);
            if ( !newResult.isComplete() ) {
                emitFailure();
                return false;
            }
            m_operator->m_outputs[0]->m_result.push_back(newResult);
        }
    }
    emitSuccess();
    return true;
}
