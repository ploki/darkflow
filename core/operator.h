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
#ifndef OPERATOR_H
#define OPERATOR_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QString>
#include <QJsonObject>

#include "ports.h"
#include "photo.h"

class OperatorParameter;
class OperatorInput;
class OperatorOutput;
class Process;
class QThread;
class OperatorWorker;

#define OP_SECTION_ASSETS           Operator::tr("Assets"), "/docs/assets.%0/#%1"
#define OP_SECTION_WORKFLOW         Operator::tr("Workflow"), "/docs/workflow.%0/#%1"
#define OP_SECTION_MASK             Operator::tr("Mask"), "/docs/mask.%0/#%1"
#define OP_SECTION_GEOMETRY         Operator::tr("Geometry"), "/docs/geometry.%0/#%1"
#define OP_SECTION_REGISTRATION     Operator::tr("Registration"), "/docs/registration.%0/#%1"
#define OP_SECTION_COLOR            Operator::tr("Color"), "/docs/color.%0/#%1"
#define OP_SECTION_CURVE            Operator::tr("Curve"), "/docs/curve.%0/#%1"
#define OP_SECTION_BLEND            Operator::tr("Blend"), "/docs/blend.%0/#%1"
#define OP_SECTION_COSMETIC         Operator::tr("Cosmetic"), "/docs/cosmetic.%0/#%1"
#define OP_SECTION_DEPRECATED       Operator::tr("Deprecated"), "/docs/deprecated.%0/#%1"
#define OP_SECTION_FREQUENCY_DOMAIN Operator::tr("Frequency Domain"), "/docs/frequency-domain.%0/#%1"
#define OP_SECTION_ANALYSIS         Operator::tr("Analysis"), "/docs/analysis.%0/#%1"


class Algorithm;

class Operator : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        OutputEnabled,
        OutputDisabled
    } OperatorOutputStatus;
    typedef enum {
        NotWaiting,
        WaitingForPlay,
        WaitingForInputs
    } WaitForParentReason;
    typedef enum {
        NA        = 0,
        Linear    = (1<<0),
        NonLinear = (1<<1),
        HDR       = (1<<2),
        NonHDR    = (Linear|NonLinear),
        All       = (NonHDR|HDR),
    } ScaleCompatibility;
    explicit Operator(const QString& classSection,
                      const char *docLink,
                      const char* classIdentifier,
                      int compatibility,
                      Process *parent);
    virtual ~Operator();

    /**
     * @brief getParameters
     * @return  the parameters of the operator
     */
    QVector<OperatorParameter*> getParameters();

    /**
     * @brief getInputs
     * @return input descriptions
     */
    QVector<OperatorInput*> getInputs() const;

    QVector<OperatorOutput *> getOutputs() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isUpToDate() const;

    QString uuid() const;
    void setUuid(const QString &uuid);

    virtual Operator* newInstance() = 0;

    virtual bool isDeprecated() const;
    virtual bool isParametric() const;
    virtual QString getGenericName() const;
    virtual int minNumbersOfWays() const;
    virtual int maxNumbersOfWays() const;
    int askForNumberOfWays(const QString& title, const QString& label) const;
    virtual Operator* newParameterizedInstance(int i);

    QString getLocalizedClassIdentifier() const;
    QString getClassIdentifier() const;
    QString getClassSection() const;
    QString getDocLink() const;

    static void operator_connect(Operator *outputOperator, int outputIdx,
                                 Operator *inputOperator, int inputIdx);
    static void operator_disconnect(Operator *outputOperator, int outputIdx,
                                    Operator *inputOperator, int inputIdx);

    void save(QJsonObject& obj, const QString& baseDirStr);
    void load(QJsonObject& obj);

    QString getName() const;

    bool spotLoop(const QString& uuid);

    bool play_parentDirty(WaitForParentReason reason);
    void addInput(OperatorInput* input);
    void addOutput(OperatorOutput* output);
    void addParameter(OperatorParameter* parameter);
    void setOutputStatus(int idx, OperatorOutputStatus status);
    bool isCompatible(const Photo& photo) const;
    bool isCompatible(const ScaleCompatibility& comp) const;

    virtual Algorithm *getAlgorithm() const;
    virtual void releaseAlgorithm(Algorithm *) const;

private:
    QVector<QVector<Photo> > collectInputs();

signals:
    void progress(int ,int );
    void upToDate();
    void outOfDate();
    void stateChanged();
    void setError(const QString& photoIdentity, const QString& msg);

public:
    virtual OperatorWorker* newWorker() = 0;

public slots:
    void play();
    void stop();
    void clone();
    void refreshInputs();
    void workerProgress(int p, int c);
    void workerSuccess(QVector<QVector<Photo> > result);
    void workerFailure();
    void parentUpToDate();
    void setName(const QString &name);
    void setUpToDate();
    void setOutOfDate();
    void setErrorTag(const QString& photoIdentity, const QString& msg);
    void setTagOverride(const QString& photoIdentity,
                        const QString& key,
                        const QString& value);
    void resetTagOverride(const QString& photoIdentity,
                          const QString& key);
    bool isTagOverrided(const QString& photoIdentity,
                        const QString& key);
    QString getTagOverrided(const QString& photoIdentity,
                            const QString& key);
    bool photoTagsExists(const QString& photoIdentity);
    QMap<QString,QString> photoTags(const QString& photoIdentity);
    void overrideTags(Photo& photo);

protected:
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


protected:
    friend class Visualization;
    Process *m_process;
    bool m_enabled;
    bool m_upToDate;
    bool m_workerAboutToStart;
private:
    QVector<OperatorParameter*> m_parameters;
    QVector<OperatorInput*> m_inputs;
    QVector<OperatorOutput*> m_outputs;
    QVector<OperatorOutputStatus> m_outputStatus;
    ScaleCompatibility m_scaleCompatibility;
protected:
    WaitForParentReason m_waitingParentFor;
    QString m_uuid;
    QString m_docLink;
    QString m_classSection;
    QString m_classIdentifier;
    QString m_localizedClassIdentifier;
    QString m_name;
    QMap<QString, QMap<QString, QString> > m_tagsOverride;

    QThread *m_thread;
    OperatorWorker *m_worker;

};



#endif // OPERATOR_H
