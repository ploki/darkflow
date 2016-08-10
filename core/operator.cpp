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
#include <QJsonArray>
#include <QStringList>
#include <QApplication>

#include <cstdio>

#include "console.h"
#include "operator.h"
#include "process.h"
#include "photo.h"
#include "operatorparameter.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"

Operator::Operator(const QString& classSection,
                   const char* classIdentifier,
                   int scaleCompatibility,
                   Process *parent) :
    QObject(NULL),
    m_process(parent),
    m_enabled(true),
    m_upToDate(false),
    m_workerAboutToStart(false),
    m_parameters(),
    m_inputs(),
    m_outputs(),
    m_outputStatus(),
    m_scaleCompatibility(ScaleCompatibility(scaleCompatibility)),
    m_waitingParentFor(NotWaiting),
    m_uuid(Process::uuid()),
    m_classSection(classSection),
    m_classIdentifier(classIdentifier),
    m_localizedClassIdentifier(tr(classIdentifier)),
    m_name(tr(classIdentifier)),
    m_tagsOverride(),
    m_thread(new QThread(this)),
    m_worker(NULL)
{
    connect(this, SIGNAL(setError(QString,QString)), this, SLOT(setErrorTag(QString,QString)), Qt::QueuedConnection);
}

Operator::~Operator()
{
    foreach(OperatorParameter *p, m_parameters)
        delete p;
    foreach(OperatorInput *i, m_inputs)
        delete i;
}

QVector<OperatorParameter *> Operator::getParameters()
{
    return m_parameters;
}

QVector<OperatorInput *> Operator::getInputs() const
{
    return m_inputs;
}

QVector<OperatorOutput *> Operator::getOutputs() const
{
    return m_outputs;
}

void Operator::stop()
{
    m_thread->requestInterruption();
}

void Operator::clone()
{
    Operator *op = newInstance();
    m_process->addOperator(op);
}

void Operator::refreshInputs()
{
    if ( isUpToDate() )
        return;
    play_parentDirty(WaitingForInputs);
    if ( m_waitingParentFor != WaitingForInputs )
        emit stateChanged();
}

void Operator::workerProgress(int p, int c)
{
    emit progress(p,c);
}

void Operator::workerSuccess(QVector<QVector<Photo> > result)
{
    m_thread->quit();
    m_worker=NULL;
    m_waitingParentFor = NotWaiting;

    int idx = 0;
    Q_ASSERT(m_outputs.count() == result.count());
    foreach(OperatorOutput *output, m_outputs) {
        output->m_result.clear();
        if ( m_outputStatus[idx] == OutputEnabled )
            foreach(Photo photo, result[idx]) {
                output->m_result.push_back(photo);
            }
        ++idx;
    }

    setUpToDate();
}

void Operator::workerFailure()
{
    m_thread->quit();
    m_worker=NULL;
    m_waitingParentFor = NotWaiting;
    setOutOfDate();
}

void Operator::parentUpToDate()
{
    switch (m_waitingParentFor) {
    case WaitingForInputs:
        emit stateChanged();
        play_parentDirty(WaitingForInputs);
        break;
    case WaitingForPlay:
        play();
        break;
    default:
        dflWarning(tr("Unknown waiting reason"));
    case NotWaiting:
        // emit permit to notif following operator of new inputs even if not waiting
        emit stateChanged();
        break;
    }
}

QString Operator::getName() const
{
    return m_name;
}

bool Operator::spotLoop(const QString &uuid)
{
    if ( m_uuid == uuid )
        return true;
    foreach(OperatorOutput *output, m_outputs) {
        foreach(OperatorInput *input, output->sinks()) {
            bool spotted = input->m_operator->spotLoop(uuid);
            if ( spotted )
                return true;
        }
    }

    return false;
}

void Operator::setName(const QString &name)
{
    m_name = name;
}

QString Operator::getClassIdentifier() const
{
    return m_classIdentifier;
}
QString Operator::uuid() const
{
    return m_uuid;
}

void Operator::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString Operator::getLocalizedClassIdentifier() const
{
    return m_localizedClassIdentifier;
}


bool Operator::play_parentDirty(WaitForParentReason reason)
{
    bool dirty = false;
    foreach(OperatorInput *input, m_inputs)
        foreach(OperatorOutput *parentOutput, input->sources()) {
            if ( !parentOutput->m_operator->isUpToDate() ) {
                dirty = true;
                parentOutput->m_operator->play();
                m_waitingParentFor = reason;
            }
        }
    if (dirty && reason == WaitingForPlay ) {
        //will be signaled later
        emit progress(0, 1);
    }
    else if ( !dirty && reason == WaitingForInputs ) {
        m_waitingParentFor = NotWaiting;
    }
    return dirty;
}

void Operator::addInput(OperatorInput *input)
{
    m_inputs.push_back(input);
}

void Operator::addOutput(OperatorOutput *output)
{
    m_outputs.push_back(output);
    m_outputStatus.push_back(OutputEnabled);
}

void Operator::addParameter(OperatorParameter *parameter)
{
    m_parameters.push_back(parameter);
}

void Operator::setOutputStatus(int idx, Operator::OperatorOutputStatus status)
{
    m_outputStatus[idx] = status;
}

bool Operator::isCompatible(const Photo &photo) const
{
    if ( photo.getScale() == Photo::Linear && (Linear & m_scaleCompatibility) )
        return true;
    else if ( photo.getScale() == Photo::NonLinear && (NonLinear & m_scaleCompatibility) )
        return true;
    else if ( photo.getScale() == Photo::HDR && (HDR & m_scaleCompatibility) )
        return true;
    return false;
}

bool Operator::isCompatible(const Operator::ScaleCompatibility &comp) const
{
    return ( comp & m_scaleCompatibility);
}

Algorithm *Operator::getAlgorithm() const
{
    return NULL;
}

void Operator::releaseAlgorithm(Algorithm *) const
{
}

QVector<QVector<Photo> > Operator::collectInputs()
{
    QMap<QString, int> seen;

    QVector<QVector<Photo> > inputs;
    int i = 0;
    foreach(OperatorInput *input, m_inputs) {
        inputs.push_back(QVector<Photo>());
        foreach(OperatorOutput *source, input->sources()) {
            foreach(Photo photo, source->m_result) {
                QString identity = photo.getIdentity();
                identity = identity.split('|').first();
                int count = ++seen[identity];
                if ( count > 1 )
                    identity+=QString("|%0").arg(count-1);
                photo.setIdentity(identity);

                overrideTags(photo);
                QString treatTag = photo.getTag(TAG_TREAT);
                if ( treatTag != TAG_TREAT_DISCARDED &&
                     treatTag != TAG_TREAT_ERROR) {
                    inputs[i].push_back(photo);
                }
                else if ( treatTag == TAG_TREAT_ERROR ) {
                    dflWarning(tr("Photo: %0 discarded because of error").arg(photo.getIdentity()));
                }
            }
        }
        ++i;
    }
    return inputs;
}

void Operator::play() {
    Q_ASSERT(QThread::currentThread() == thread());
    if (m_worker) {
        dflDebug(tr("Already playing"));
        return;
    }
    if (isUpToDate())
        return;
    if (play_parentDirty(WaitingForPlay))
        return;
    dflDebug("play on "+m_uuid);
    m_workerAboutToStart = true;
    m_worker = newWorker();
    setOutOfDate();
    m_worker->start(collectInputs(), m_outputStatus);
    m_workerAboutToStart = false;
    dflDebug(tr("Worker started for %0").arg(m_uuid));
}

bool Operator::isUpToDate() const
{
    dflDebug(tr("%0 is up to date: %1").arg(m_uuid).arg(m_upToDate && !m_worker));
    return m_upToDate;
}

void Operator::setUpToDate()
{
    Q_ASSERT(QThread::currentThread() == thread());
    m_upToDate = true;
    emit progress(1, 1);
    emit upToDate();
}

void Operator::setOutOfDate()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if ( m_worker && !m_workerAboutToStart ) {
        dflDebug(tr("Sending 'stop' to worker"));
        stop();
        return;
    }
    m_upToDate = false;
    foreach(OperatorOutput *output, m_outputs) {
        output->m_result.clear();
        foreach(OperatorInput *remoteInput, output->sinks())
            remoteInput->m_operator->setOutOfDate();
    }
    if ( !m_workerAboutToStart )
        emit outOfDate();
    emit progress(0, 1);
}

void Operator::setErrorTag(const QString &photoIdentity, const QString &msg)
{
    setTagOverride(photoIdentity, TAG_TREAT, TAG_TREAT_ERROR);
    setTagOverride(photoIdentity, TAG_TREAT_ERROR, msg);
}

void Operator::setTagOverride(const QString &photoIdentity, const QString &key, const QString &value)
{
    m_tagsOverride[photoIdentity].insert(key,value);
    setOutOfDate();
}

void Operator::resetTagOverride(const QString &photoIdentity, const QString &key)
{
    m_tagsOverride[photoIdentity].remove(key);
    if ( m_tagsOverride[photoIdentity].empty() )
        m_tagsOverride.remove(photoIdentity);
    setOutOfDate();
}

bool Operator::isTagOverrided(const QString &photoIdentity, const QString &key)
{
    if ( m_tagsOverride.find(photoIdentity) == m_tagsOverride.end() )
        return false;
    if ( m_tagsOverride[photoIdentity].find(key) == m_tagsOverride[photoIdentity].end() )
        return false;
    return true;
}

QString Operator::getTagOverrided(const QString &photoIdentity, const QString &key)
{
    return m_tagsOverride[photoIdentity][key];
}

bool Operator::photoTagsExists(const QString &photoIdentity)
{
    return m_tagsOverride.find(photoIdentity) != m_tagsOverride.end();
}

QMap<QString, QString> Operator::photoTags(const QString &photoIdentity)
{
    return m_tagsOverride[photoIdentity];
}

/**
 * @brief Operator::overrideTags
 * @param photo with identity's dedup postfix
 */
void Operator::overrideTags(Photo &photo)
{
    QString identity = photo.getIdentity();
    if (!photoTagsExists(identity))
        return;
    QMap<QString, QString> tags = photoTags(identity);
    for(QMap<QString, QString>::iterator it = tags.begin() ;
        it != tags.end() ;
        ++it ) {
        if ( it.value().count() == 0 )
            photo.removeTag(it.key());
        else
            photo.setTag(it.key(), it.value());
    }
}

QString Operator::getClassSection() const
{
    return m_classSection;
}

bool Operator::isEnabled() const
{
    return m_enabled;
}

void Operator::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void Operator::operator_connect(Operator *outputOperator, int outputIdx,
                                Operator *inputOperator, int inputIdx)
{
    OperatorOutput *output = outputOperator->m_outputs[outputIdx];
    OperatorInput *input = inputOperator->m_inputs[inputIdx];
    output->addSink(input);
    input->addSource(output);
    connect(output->m_operator, SIGNAL(upToDate()), input->m_operator, SLOT(parentUpToDate()));
    input->m_operator->setOutOfDate();
    emit inputOperator->stateChanged();
}

void Operator::operator_disconnect(Operator *outputOperator, int outputIdx,
                                   Operator *inputOperator, int inputIdx)
{
    OperatorOutput *output = outputOperator->m_outputs[outputIdx];
    OperatorInput *input = inputOperator->m_inputs[inputIdx];
    disconnect(output->m_operator, SIGNAL(upToDate()), input->m_operator, SLOT(parentUpToDate()));
    output->removeSink(input);
    input->removeSource(output);
    input->m_operator->setOutOfDate();
    emit inputOperator->stateChanged();
}

void Operator::save(QJsonObject &obj, const QString& baseDirStr)
{
    QJsonArray parameters;
    obj["enabled"] = m_enabled;
    obj["uuid"] = m_uuid;
    obj["classIdentifier"] = getClassIdentifier();
    obj["name"] = getName();
    foreach(OperatorParameter *parameter, m_parameters) {
        dflDebug(tr("Saving a parameter"));
        parameters.push_back(parameter->save(baseDirStr));
    }
    obj["parameters"] = parameters;

    QJsonObject allTags;
    for(QMap<QString, QMap<QString, QString> >::iterator it = m_tagsOverride.begin() ;
        it != m_tagsOverride.end() ;
        ++it ) {
        QJsonObject photoTags;
        for(QMap<QString, QString>::iterator tag = it.value().begin() ;
            tag != it.value().end() ;
            ++tag ) {
            photoTags[tag.key()] = tag.value();
        }
        allTags[it.key()]=photoTags;
    }
    obj["tags"] = allTags;
    QJsonArray outputsEnabled;
    for(int i = 0 ; i < m_outputStatus.count() ; ++i ) {
        outputsEnabled.push_back(m_outputStatus[i]==OutputEnabled);
    }
    obj["outputsEnabled"] = outputsEnabled;
}

void Operator::load(QJsonObject &obj)
{
    QJsonArray array = obj["parameters"].toArray();
    m_name = obj["name"].toString();
    m_enabled = obj["enabled"].toBool();
    m_uuid = obj["uuid"].toString();
    for (int i = 0 ; i < m_parameters.count(); ++i ) {
        QJsonObject obj = array[i].toObject();
        m_parameters[i]->load(obj);
    }
    m_tagsOverride.clear();
    QJsonObject tags = obj["tags"].toObject();
    for (QJsonObject::iterator it = tags.begin() ;
         it != tags.end() ;
         ++it ) {
        QMap<QString, QString> photoTags;
        QJsonObject photoTagsObject = it.value().toObject();
        for (QJsonObject::iterator tag = photoTagsObject.begin() ;
             tag != photoTagsObject.end() ;
             ++tag ) {
            photoTags[tag.key()] = tag.value().toString();
        }
        m_tagsOverride[it.key()] = photoTags;
    }
    QJsonArray outputsEnabled = obj["outputsEnabled"].toArray();
    for (int i = 0 ; i < outputsEnabled.count() ; ++i ) {
        m_outputStatus[i] = outputsEnabled[i].toBool() ? OutputEnabled : OutputDisabled;
    }
    emit stateChanged();
}


static void logMessage(Console::Level level, const QString& who, const QString& msg)
{
    dflMessage(level, who+": "+msg);
}

void Operator::dflDebug(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Debug, getName(), msg);
    free(msg);
}

void Operator::dflInfo(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Info, getName(), msg);
    free(msg);
}

void Operator::dflWarning(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Warning, getName(), msg);
    free(msg);
}

void Operator::dflError(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Error, getName(), msg);
    free(msg);
}

void Operator::dflCritical(const char *fmt, ...) const
{
    va_list ap;
    char *msg;
    int ret;
    va_start(ap, fmt);
    ret = vasprintf(&msg, fmt, ap);
    va_end(ap);
    if ( ret < 0 ) return;
    logMessage(Console::Critical, getName(), msg);
    free(msg);
}

void Operator::dflDebug(const QString &msg) const
{
    logMessage(Console::Debug, getName(), msg);
}

void Operator::dflInfo(const QString &msg) const
{
    logMessage(Console::Info, getName(), msg);
}

void Operator::dflWarning(const QString &msg) const
{
    logMessage(Console::Warning, getName(), msg);
}

void Operator::dflError(const QString &msg) const
{
    logMessage(Console::Error, getName(), msg);
}

void Operator::dflCritical(const QString &msg) const
{
    logMessage(Console::Critical, getName(), msg);
}
