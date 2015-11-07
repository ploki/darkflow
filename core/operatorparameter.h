#ifndef OPERATORPARAMETER_H
#define OPERATORPARAMETER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

class OperatorParameter : public QObject
{
    Q_OBJECT
public:
    explicit OperatorParameter(const QString& name,
                               const QString& caption,
                               QObject *parent = 0);

    virtual QJsonObject save() = 0;
    virtual void load(const QJsonObject& obj) = 0;

    QString caption() const;

    QString name() const;

signals:
    void parameterChanged();

public slots:
protected:
    QString m_name;
    QString m_caption;
};

#endif // OPERATORPARAMETER_H
