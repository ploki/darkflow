/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
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
        dflCritical(tr("OutputPush idx out of range"));
    }
}

void OperatorWorker::outputSort(int idx)
{
    if ( idx < m_outputs.count() ) {
        if ( m_outputStatus[idx] == Operator::OutputEnabled )
            qSort(m_outputs[idx]);
    }
    else {
        dflCritical(tr("outputSort idx out of range"));
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
        dflDebug(tr("OperatorWorker::finished: no signal sent, sending failure"));
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
    emit progress(0, 1);
    emit failure();
    if ( ( m_earlyAbort || aborted() ) && !m_error )
        dflDebug(tr("Aborted (after %0ms)").arg(m_elapsed.elapsed()));
    else
        dflError(tr("Failure (after %0ms)").arg(m_elapsed.elapsed()));
}

void OperatorWorker::emitSuccess()
{
    m_signalEmited = true;
    emit progress(1, 1);
    emit success(m_outputs);
    dflInfo(tr("Success (after %0ms)").arg(m_elapsed.elapsed()));
}

void OperatorWorker::emitProgress(int p, int c, int sub_p, int sub_c)
{
    emit progress( p * sub_c + sub_p , c * sub_c);
}

bool OperatorWorker::play_inputsAvailable()
{
    if ( 0 == m_inputs.size() ) {
        dflCritical(tr("OperatorWorker::play() not overloaded for an operator without inputs"));
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
        dflCritical(tr("OperatorWorker::play() not overloaded for #output != 1"));
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
            dflDebug(tr("In error, sending failure"));
            emitFailure();
            return false;
        }
        if ( aborted() ) {
            dflDebug(tr("Aborted, sending failure"));
            emitFailure();
            return false;
        }
        emit progress(p, c);
        try {
            Photo newPhoto;
            if ( !m_operator->isCompatible(photo) ) {
                switch ( preferences->getIncompatibleAction()) {
                case Preferences::Warning:
                    dflWarning(tr("Incompatible pixel scale: %0").arg(photo.getIdentity()));
                    //Falls through
                case Preferences::Ignore:
                    if ( photo.getScale() == Photo::HDR )
                        HDR(true).applyOn(photo);
                    break;
                default:
                case Preferences::Error:
                    setError(photo, tr("Incompatible pixel scale"));
                    break;
                }
            }
            if (!m_error) {
                newPhoto = this->process(photo, p++, c);
                if ( !newPhoto.isComplete() ) {
                    dflDebug(tr("Photo is not complete, sending failure"));
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
    dfl_block int p = 0;
    c = m_inputs[idx].count();
    dfl_parallel_for(i, 0, c, 1, (), {
        if ( m_error || aborted() )
            continue;
        Photo photo;

        dfl_critical_section({
            photo = m_inputs[idx][i];
        });
        Photo newPhoto;
        if ( !m_operator->isCompatible(photo) ) {
            switch ( preferences->getIncompatibleAction()) {
            case Preferences::Warning:
                dflWarning(tr("Incompatible pixel scale, converted").arg(photo.getIdentity()));
                // Falls through
            case Preferences::Ignore:
                if ( photo.getScale() == Photo::HDR )
                    HDR(true).applyOn(photo);
                break;
            default:
            case Preferences::Error:
                setError(photo, tr("Incompatible pixel scale"));
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
                dflDebug(tr("Photo is not complete, sending failure"));
                m_error = true;
                continue;
            }
            newPhoto.setSequenceNumber(i);
        }
        dfl_critical_section({
            if ( !m_error ) {
                emit progress(++p, c);
                m_outputs[0].push_back(newPhoto);
            }
        });
    });
    if ( m_error ) {
        dflDebug(tr("In error, sending failure"));
        emitFailure();
    }
    else if ( aborted() ) {
        dflDebug(tr("Aborted, sending failure"));
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
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Debug, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflInfo(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Info, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflWarning(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Warning, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflError(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Error, m_operator->getName(), msg);
    free(msg);
}

void OperatorWorker::dflCritical(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
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
    dflError(tr("Photo: %0, Error: %1").arg(photo.getIdentity()).arg(msg));
    emit m_operator->setError(photo.getIdentity(), msg);
    m_error = true;
    m_operator->stop();
}
