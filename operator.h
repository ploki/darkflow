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
    double getCompletion();

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

public slots:
    virtual void play();
    void abort();
    void clone();
protected:
    Process *m_process;
    bool m_enabled;
    bool m_upToDate;
    long m_complete;
    long m_progress;
    QVector<OperatorParameter*> m_parameters;
    QVector<OperatorInput*> m_inputs;
    QMap<QString, Operator*> m_sources;
    QSet<Operator*> m_sinks;
    QVector<Image*> m_result;

    virtual Image * process(const Image *) = 0;
};

#endif // OPERATOR_H
