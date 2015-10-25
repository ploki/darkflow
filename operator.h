#ifndef OPERATOR_H
#define OPERATOR_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>

class OperatorParameter;
class OperatorInput;
class Process;
class Image;
class QThread;
class OperatorWorker;

class Operator : public QObject
{
    Q_OBJECT
public:
    explicit Operator(Process *parent);
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
    QVector<OperatorInput*> getInputs();

    void addSource(const QString &inputName, Operator *op);
    void clearSource(const QString &inputName);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isUpToDate() const;
    void setUpToDate(bool upToDate);

    QVector<Image *> getResult() const;
    virtual QString getClassIdentifier() = 0;
    virtual Operator* newInstance() = 0;


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

//protected:
public:
    Process *m_process;
    bool m_enabled;
    bool m_upToDate;
    QVector<OperatorParameter*> m_parameters;
    QVector<OperatorInput*> m_inputs;
    QMap<QString, Operator*> m_sources;
    QSet<Operator*> m_sinks;
    QVector<Image*> m_result;

    QThread *m_thread;
    OperatorWorker *m_worker;

};




#endif // OPERATOR_H
