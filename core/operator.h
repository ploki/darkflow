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

class Operator : public QObject
{
    Q_OBJECT
public:
    explicit Operator(const QString& classIdentifier, Process *parent);
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

    void addSource(const QString &inputName, Operator *op);
    void clearSource(const QString &inputName);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isUpToDate() const;
    void setUpToDate(bool upToDate);

    QString getUuid() const;
    void setUuid(const QString &uuid);

    virtual Operator* newInstance() = 0;

    QString getClassIdentifier() const;

    static void operator_connect(OperatorOutput *output, OperatorInput *input);
    static void operator_disconnect(OperatorOutput *output, OperatorInput *input);

    virtual void save(QJsonObject& obj);
    virtual void load(QJsonObject& obj);

    QString getName() const;

    bool spotLoop(const QString& uuid);

signals:
    void progress(int ,int );
    void upToDate();
public:
    virtual OperatorWorker* newWorker() = 0;

public slots:
    void play();
    void abort();
    void clone();
    void workerProgress(int p, int c);
    void workerSuccess();
    void workerFailure();
    void parentUpToDate();
    void setName(const QString &name);

//protected:
public:
    Process *m_process;
    bool m_enabled;
    bool m_upToDate;
    QVector<OperatorParameter*> m_parameters;
    QVector<OperatorInput*> m_inputs;
    QVector<OperatorOutput*> m_outputs;
    bool m_waitingForParentUpToDate;
    QString m_uuid;
    QString m_classIdentifier;
    QString m_name;

    QThread *m_thread;
    OperatorWorker *m_worker;

};



#endif // OPERATOR_H
