#include <QThread>

#include "operatorworker.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"

static struct AtStart {
    AtStart() {
        qRegisterMetaType<QVector<QVector<Photo> > >("QVector<QVector<Photo> >");
    }
} foo;

OperatorWorker::OperatorWorker(QThread *thread, Operator *op) :
    QObject(NULL),
    m_thread(thread),
    m_operator(op),
    m_inputs(),
    m_outputs(),
    m_outputStatus(),
    m_signalEmited(false)
{
    moveToThread(thread);
    connect(m_thread, SIGNAL(finished()), this, SLOT(finished()));
    connect(this, SIGNAL(doStart()), this, SLOT(play()));
    connect(this, SIGNAL(progress(int,int)), m_operator, SLOT(workerProgress(int,int)));
    connect(this, SIGNAL(success(QVector<QVector<Photo> >)), m_operator, SLOT(workerSuccess(QVector<QVector<Photo> >)));
    connect(this, SIGNAL(failure()), m_operator, SLOT(workerFailure()));
    m_thread->start();
}

void OperatorWorker::start(QVector<QVector<Photo> > inputs, QVector<Operator::OperatorOutputStatus> outputStatus)
{
    m_inputs = inputs;
    prepareOutputs(outputStatus);
    emit doStart();
}

int OperatorWorker::outputsCount()
{
    return m_outputs.count();
}

void OperatorWorker::outputPush(int idx, const Photo &photo)
{
    if ( idx < m_outputs.count() ) {
        if ( m_outputStatus[idx] == Operator::OutputEnabled )
            m_outputs[idx].push_back(photo);
    }
    else {
        qWarning("outputPush idx out of range");
    }
}

void OperatorWorker::outputSort(int idx)
{
    if ( idx < m_outputs.count() ) {
        if ( m_outputStatus[idx] == Operator::OutputEnabled )
            qSort(m_outputs[idx]);
    }
    else {
        qWarning("outputSort idx out of range");
    }
}

void OperatorWorker::play()
{
    qDebug("OperatorWorker::play()");

    if ( !play_inputsAvailable() )
        return;
    if (!play_outputsAvailable())
        return;

    play_analyseSources();

    play_onInput(0);
    if (!m_signalEmited) {
        qWarning("BUG: No signal sent!!!");
        emitFailure();
    }
}

void OperatorWorker::finished()
{
    if ( !m_signalEmited) {
        qDebug("OperatorWorker::finished: no signal sent, sending failure");
        emitFailure();
    }
    else { //signal emited, safe to delete
        deleteLater();
    }
}

bool OperatorWorker::aborted() {
    return m_thread->isInterruptionRequested();
}

void OperatorWorker::emitFailure() {
    m_signalEmited = true;
    qDebug(QString("Worker of " + m_operator->uuid() + "emit progress(0, 1)").toLatin1());
    emit progress(0, 1);
    qDebug(QString("Worker of " + m_operator->uuid() + "emit failure").toLatin1());
    emit failure();
    qDebug(QString("Worker of " + m_operator->uuid() + "emit done").toLatin1());
}

void OperatorWorker::emitSuccess()
{
    m_signalEmited = true;
    qDebug(QString("Worker of " + m_operator->uuid() + "emit progress(1, 1)").toLatin1());
    emit progress(1, 1);
    qDebug(QString("Worker of " + m_operator->uuid() + "emit success").toLatin1());
    emit success(m_outputs);
    qDebug(QString("Worker of " + m_operator->uuid() + "emit done").toLatin1());
}

void OperatorWorker::emitProgress(int p, int c, int sub_p, int sub_c)
{
    emit progress( p * sub_c + sub_p , c * sub_c);
}

bool OperatorWorker::play_inputsAvailable()
{
    if ( 0 == m_inputs.size() ) {
        qWarning("OperatorWorker::play() not overloaded for no input");
        emitFailure();
        return false;
    }
    return true;
}

void OperatorWorker::prepareOutputs(QVector<Operator::OperatorOutputStatus> outputStatus)
{
    m_outputStatus = outputStatus;
    int n_outputs = outputStatus.count();
    for (int i = 0 ; i < n_outputs ; ++i )
        m_outputs.push_back(QVector<Photo>());
}

bool OperatorWorker::play_outputsAvailable()
{
    if ( 0 == m_outputs.count() ) {
        qWarning("OperatorWorker::play() not overloaded for #output != 1");
        emitFailure();
        return false;
    }
    return true;
}

void OperatorWorker::play_analyseSources()
{

}

bool OperatorWorker::play_onInput(int idx)
{
    int c = 0;
    int p = 0;
    c = m_inputs[idx].count();

    foreach(Photo photo, m_inputs[idx]) {
        if ( aborted() ) {
            qDebug("OperatorWorker aborted, sending failure");
            emitFailure();
            return false;
        }
        emit progress(p, c);
        Photo newPhoto = this->process(photo, p++, c);
        if ( !newPhoto.isComplete() ) {
            qWarning("OperatorWorker: photo is not complete, sending failure");
            emitFailure();
            return false;
        }
        m_outputs[0].push_back(newPhoto);
    }

    emitSuccess();
    return true;
}

bool OperatorWorker::play_onInputParallel(int idx)
{
    int c = 0;
    int p = 0;
    c = m_inputs[idx].count();

#pragma omp parallel for
    for (int i = 0 ; i < c ; ++i ) {
        if ( aborted() )
            continue;
        Photo photo;
#pragma omp critical
        {
            photo = m_inputs[idx][i];
        }
        Photo newPhoto = this->process(photo, p, c);
        if ( !newPhoto.isComplete() ) {
            qWarning("OperatorWorker: photo is not complete, sending failure");
            continue;
        }
        newPhoto.setSequenceNumber(i);

#pragma omp critical
        {
            emit progress(++p, c);
            m_outputs[0].push_back(newPhoto);
        }
    }

    if ( aborted() ) {
        qDebug("OperatorWorker aborted, sending failure");
        emitFailure();
    }
    else {
        qSort(m_outputs[idx]);
        emitSuccess();
    }
    return true;
}
