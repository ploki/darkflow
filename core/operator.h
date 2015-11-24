#ifndef OPERATOR_H
#define OPERATOR_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QString>
#include <QJsonObject>

#include "photo.h"

class OperatorParameter;
class OperatorInput;
class OperatorOutput;
class Process;
class QThread;
class OperatorWorker;

#define OP_SECTION_SOURCE_IMAGES "Source Images"
#define OP_SECTION_UTILITY "Utility"
#define OP_SECTION_GEOMETRY "Geometry"
#define OP_SECTION_CORRECTION "Correction"
#define OP_SECTION_CURVE "Curve"
#define OP_SECTION_BLEND "Blend"
#define OP_SECTION_COSMETIC "Cosmetic"
#define OP_SECTION_DEPRECATED "Deprecated"


class Operator : public QObject
{
    Q_OBJECT
public:
    explicit Operator(const QString& classSection, const QString& classIdentifier, Process *parent);
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

    QString getClassIdentifier() const;
    QString getClassSection() const;

    static void operator_connect(Operator *outputOperator, int outputIdx,
                                 Operator *inputOperator, int inputIdx);
    static void operator_disconnect(Operator *outputOperator, int outputIdx,
                                    Operator *inputOperator, int inputIdx);

    virtual void save(QJsonObject& obj);
    virtual void load(QJsonObject& obj);

    QString getName() const;

    bool spotLoop(const QString& uuid);

    bool play_parentDirty();

private:
    QVector<QVector<Photo> > collectInputs();

signals:
    void progress(int ,int );
    void upToDate();
    void outOfDate();

public:
    virtual OperatorWorker* newWorker() = 0;

public slots:
    void play();
    void stop();
    void clone();
    void workerProgress(int p, int c);
    void workerSuccess(QVector<QVector<Photo> > result);
    void workerFailure();
    void parentUpToDate();
    void setName(const QString &name);
    void setUpToDate();
    void setOutOfDate();
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
    friend class Visualization;
    Process *m_process;
    bool m_enabled;
    bool m_upToDate;
    bool m_workerAboutToStart;
    QVector<OperatorParameter*> m_parameters;
    QVector<OperatorInput*> m_inputs;
    QVector<OperatorOutput*> m_outputs;
    bool m_waitingForParentUpToDate;
    QString m_uuid;
    QString m_classSection;
    QString m_classIdentifier;
    QString m_name;
    QMap<QString, QMap<QString, QString> > m_tagsOverride;

    QThread *m_thread;
    OperatorWorker *m_worker;

};



#endif // OPERATOR_H
