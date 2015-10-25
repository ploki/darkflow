#include <QThread>

#include "operatorworker.h"
#include "operator.h"
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
    foreach(Operator *op, m_operator->m_sources ) {
        qWarning("OperatorWorker::play():one source!");
        if ( !op->isUpToDate() ) {
            parentDirty = true;
            op->play();
            connect(op, SIGNAL(upToDate()), this, SLOT(play()));
        }
    }
    if ( parentDirty )
        //will be signaled later
        return;

    if ( !parentDirty && m_operator->isUpToDate())
    {
        emit success();
        return;
    }

    m_operator->setUpToDate(false);
    foreach(Operator *op, m_operator->m_sources ) {
        const QVector<Image*> source = op->getResult();
        foreach(const Image *image, source) {
            Image *newResult = process(image);
            m_operator->m_result.push_back(newResult);
        }
    }

}

bool OperatorWorker::aborted() {
    return QThread::currentThread()->isInterruptionRequested();
}

void OperatorWorker::emitFailure() {
    emit progress(0, 1); emit failure();
}
