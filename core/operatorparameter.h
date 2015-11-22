#ifndef OPERATORPARAMETER_H
#define OPERATORPARAMETER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

#include "operator.h"

class OperatorParameter : public QObject
{
    Q_OBJECT
public:
    OperatorParameter(const QString& name,
                      const QString& caption,
                      Operator *op);

    virtual QJsonObject save() = 0;
    virtual void load(const QJsonObject& obj) = 0;

    QString caption() const;

    QString name() const;

signals:
    void parameterChanged();
    void setOutOfDate();

public slots:
protected:
    Operator *m_operator;
    QString m_name;
    QString m_caption;
};

#endif // OPERATORPARAMETER_H
