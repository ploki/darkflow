#ifndef OPERATORPARAMETERFILESCOLLECTION_H
#define OPERATORPARAMETERFILESCOLLECTION_H

#include <QString>
#include <QStringList>
#include "operatorparameter.h"

class OperatorParameterFilesCollection : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterFilesCollection(
            const QString& name,
            const QString& caption,
            const QString& windowCaption,
            const QString& dir,
            const QString& filter,
            Operator *op);
    QString windowCaption() const;

    QString dir() const;

    QString filter() const;

    QStringList collection() const;
    void setCollection(const QStringList &collection);
    QString currentValue() const;

    QJsonObject save();
    void load(const QJsonObject &obj);

signals:
    void updated();

private:
    QString m_windowCaption;
    QString m_dir;
    QString m_filter;
    QSet<QString> m_collection;
};

#endif // OPERATORPARAMETERFILESCOLLECTION_H
