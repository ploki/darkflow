#include <QThread>

#include <cstdio>

#include "console.h"
#include "operatorworker.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "preferences.h"
#include "hdr.h"

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
    m_elapsed(),
    m_signalEmited(false),
    m_error(false),
    m_earlyAbort(false)
{
    moveToThread(thread);
    connect(m_thread, SIGNAL(finished()), this, SLOT(finished()));
    connect(this, SIGNAL(doStart()), this, SLOT(started()));
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
void OperatorWorker::started()
{
    bool ret = preferences->acquireWorker(this);
    if ( !ret ) {
        emitFailure();
        return;
    }
    m_elapsed.start();
    play();
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
        dflWarning("outputPush idx out of range");
    }
}

void OperatorWorker::outputSort(int idx)
{
    if ( idx < m_outputs.count() ) {
        if ( m_outputStatus[idx] == Operator::OutputEnabled )
            qSort(m_outputs[idx]);
    }
    else {
        dflWarning("outputSort idx out of range");
    }
}

void OperatorWorker::play()
{
    dflDebug("OperatorWorker::play()");

    if ( !play_inputsAvailable() )
        return;
    if (!play_outputsAvailable())
        return;

    play_analyseSources();

    play_onInput(0);
    if (!m_signalEmited) {
        dflCritical("BUG: No signal sent!!!");
        emitFailure();
    }
}

void OperatorWorker::finished()
{
    if ( !m_signalEmited) {
        dflDebug("OperatorWorker::finished: no signal sent, sending failure");
        m_earlyAbort = true;
        emitFailure();
    }
    else { //signal emited, safe to delete
        deleteLater();
    }
    preferences->releaseWorker();
}

bool OperatorWorker::aborted() {
    return m_thread->isInterruptionRequested();
}

void OperatorWorker::emitFailure() {
    m_signalEmited = true;
    dflDebug(QString("Worker of " + m_operator->uuid() + "emit progress(0, 1)"));
    emit progress(0, 1);
    dflDebug(QString("Worker of " + m_operator->uuid() + "emit failure"));
    emit failure();
    dflDebug(QString("Worker of " + m_operator->uuid() + "emit done"));
    if ( ( m_earlyAbort || aborted() ) && !m_error )
        dflDebug("Aborted (after %lldms)",m_elapsed.elapsed());
    else
        dflError("Failure (after %lldms)",m_elapsed.elapsed());
}

void OperatorWorker::emitSuccess()
{
    m_signalEmited = true;
    dflDebug(QString("Worker of " + m_operator->uuid() + "emit progress(1, 1)"));
    emit progress(1, 1);
    dflDebug(QString("Worker of " + m_operator->uuid() + "emit success"));
    emit success(m_outputs);
    dflDebug(QString("Worker of " + m_operator->uuid() + "emit done"));
    dflInfo("Success (after %lldms)",m_elapsed.elapsed());
}

void OperatorWorker::emitProgress(int p, int c, int sub_p, int sub_c)
{
    emit progress( p * sub_c + sub_p , c * sub_c);
}

bool OperatorWorker::play_inputsAvailable()
{
    if ( 0 == m_inputs.size() ) {
        dflCritical("OperatorWorker::play() not overloaded for no input");
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
        dflCritical("OperatorWorker::play() not overloaded for #output != 1");
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
        if ( m_error ) {
            dflDebug("in error, sending failure");
            emitFailure();
            return false;
        }
        if ( aborted() ) {
            dflDebug("aborted, sending failure");
            emitFailure();
            return false;
        }
        emit progress(p, c);
        try {
            Photo newPhoto;
            if ( !m_operator->isCompatible(photo) ) {
                switch ( preferences->getIncompatibleAction()) {
                case Preferences::Warning:
                    dflWarning(photo.getIdentity() + ": Incompatible pixel scale");
                case Preferences::Ignore:
                    if ( photo.getScale() == Photo::HDR )
                        HDR(true).applyOn(photo);
                    break;
                default:
                case Preferences::Error:
                    setError(photo, "Incompatible pixel scale");
                    break;
                }
            }
            if (!m_error) {
                newPhoto = this->process(photo, p++, c);
                if ( !newPhoto.isComplete() ) {
                    dflDebug("photo is not complete, sending failure");
                    m_error = true;
                    emitFailure();
                    return false;
                }
            }
            if ( !m_error )
                m_outputs[0].push_back(newPhoto);
        }
        catch(std::exception &e) {
            setError(photo, e.what());
            emitFailure();
            return false;
        }
    }

    if ( m_error || aborted() ) {
        emitFailure();
        return false;
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
        if ( m_error || aborted() )
            continue;
        Photo photo;
#pragma omp critical
        {
            photo = m_inputs[idx][i];
        }
        Photo newPhoto;
        if ( !m_operator->isCompatible(photo) ) {
            switch ( preferences->getIncompatibleAction()) {
            case Preferences::Warning:
                dflWarning(photo.getIdentity() + ": Incompatible pixel scale, converted");
            case Preferences::Ignore:
                if ( photo.getScale() == Photo::HDR )
                    HDR(true).applyOn(photo);
                break;
            default:
            case Preferences::Error:
                setError(photo, "Incompatible pixel scale");
                break;
            }
        }
        if (!m_error) {
            try {
                newPhoto = this->process(photo, p, c);
            }
            catch (std::exception &e) {
                setError(photo, e.what());
                continue;
            }
            if ( !newPhoto.isComplete() ) {
                dflDebug("photo is not complete, sending failure");
                m_error = true;
                continue;
            }
            newPhoto.setSequenceNumber(i);
        }
#pragma omp critical
        {
            if ( !m_error ) {
                emit progress(++p, c);
                m_outputs[0].push_back(newPhoto);
            }
        }
    }
    if ( m_error ) {
        dflDebug("in error, sending failure");
        emitFailure();
    }
    else if ( aborted() ) {
        dflDebug("aborted, sending failure");
        emitFailure();
    }
    else {
        qSort(m_outputs[idx]);
        emitSuccess();
    }
    return true;
}

static void logMessage(Console::Level level, const QString& who, const QString& msg)
{
    dflMessage(level, who+"(Worker): "+msg);
}

void OperatorWorker::dflDebug(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);
    logMessage(Console::Debug, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflInfo(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);
    logMessage(Console::Info, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflWarning(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);
    logMessage(Console::Warning, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflError(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);
    logMessage(Console::Error, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflCritical(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);
    logMessage(Console::Critical, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflDebug(const QString &msg) const
{
    logMessage(Console::Debug, m_operator->getName(), msg);
}

void OperatorWorker::dflInfo(const QString &msg) const
{
    logMessage(Console::Info, m_operator->getName(), msg);
}

void OperatorWorker::dflWarning(const QString &msg) const
{
    logMessage(Console::Warning, m_operator->getName(), msg);
}

void OperatorWorker::dflError(const QString &msg) const
{
    logMessage(Console::Error, m_operator->getName(), msg);
}

void OperatorWorker::dflCritical(const QString &msg) const
{
    logMessage(Console::Critical, m_operator->getName(), msg);
}

void OperatorWorker::setError(const Photo &photo, const QString &msg) const
{
    dflError("Photo: " + photo.getIdentity() + ", Error: " + msg);
    emit m_operator->setError(photo.getIdentity(), msg);
    m_error = true;
    m_operator->stop();
}
