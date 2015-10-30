#include <QThread>

#include "operatorworker.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "image.h"


OperatorWorker::OperatorWorker(QThread *thread, Operator *op) :
    QObject(NULL),
    m_thread(thread),
    m_operator(op)
{
    moveToThread(thread);
    connect(m_thread, SIGNAL(finished()), this, SLOT(deleteLater()));
    connect(this, SIGNAL(start()), this, SLOT(play()));
    connect(this, SIGNAL(progress(int,int)), m_operator, SLOT(workerProgress(int,int)));
    connect(this, SIGNAL(success()), m_operator, SLOT(workerSuccess()));
    connect(this, SIGNAL(failure()), m_operator, SLOT(workerFailure()));
    m_thread->start();
    emit start();
}

void OperatorWorker::play()
{
    bool parentDirty = false;
    qWarning("OperatorWorker::play()");
    if ( 0 == m_operator->m_inputs.size() ) {
        qWarning("OperatorWorker::play() not overloaded for no input");
        emitFailure();
        return;
    }
    if ( 0 == m_operator->m_outputs.size() ) {
        qWarning("OperatorWorker::play() not overloaded for #output != 1");
        emitFailure();
        return;
    }

    foreach(OperatorInput *input, m_operator->m_inputs)
        foreach(OperatorOutput *remoteOutput, input->sources()) {
            if ( !remoteOutput->m_operator->isUpToDate() ) {
                parentDirty = true;
                remoteOutput->m_operator->play();
                m_operator->m_waitingForParentUpToDate = true;
            }

        }

    if ( parentDirty ) {
        //will be signaled later
        emitFailure();
        return;
    }

    if ( !parentDirty && m_operator->isUpToDate())
    {
        emitSuccess();
        return;
    }

    m_operator->setUpToDate(false);
    foreach(OperatorOutput *remoteOutput, m_operator->m_inputs[0]->sources()) {
        const QVector<Image*> source = remoteOutput->m_result;
        foreach(const Image *image, source) {
            Image *newResult = process(image);
            m_operator->m_outputs[0]->m_result.push_back(newResult);
        }
    }
    emitSuccess();
}

bool OperatorWorker::aborted() {
    return QThread::currentThread()->isInterruptionRequested();
}

void OperatorWorker::emitFailure() {
    emit progress(0, 1); emit failure();
}

void OperatorWorker::emitSuccess()
{
    emit progress(1, 1); emit success();
}
