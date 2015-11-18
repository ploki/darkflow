#ifndef OPERATOROUTPUT_H
#define OPERATOROUTPUT_H

#include <QVector>
#include <QObject>
#include <QString>
#include <QSet>

#include "photo.h"

class Operator;
class OperatorInput;

class OperatorOutput : public QObject
{
    Q_OBJECT
public:
    explicit OperatorOutput(const QString& name,
                            const QString& description,
                            Operator *parent = 0);
    ~OperatorOutput();

    QString name() const;

    QString description() const;

    QSet<OperatorInput *> sinks() const;
    void addSink(OperatorInput *input);
    void removeSink(OperatorInput *input);
    QVector<Photo> getResult() const;

public:
    Operator *m_operator;
private:
    QString m_name;
    QString m_description;
    QSet<OperatorInput*> m_sinks;
public:
    QVector<Photo> m_result;

public slots:
};

#endif // OPERATOROUTPUT_H
