#ifndef OPERATORPARAMETERFILESCOLLECTION_H
#define OPERATORPARAMETERFILESCOLLECTION_H

#include <QString>
#include <QStringList>
#include "operatorparameter.h"

class OperatorParameterFilesCollection : public OperatorParameter
{
public:
    OperatorParameterFilesCollection(const QString& caption,
                                     const QString& windowCaption,
                                     const QString& dir,
                                     const QString& filter,
                                     QObject *parent = 0);
    QString caption() const;
    QString windowCaption() const;

    QString dir() const;

    QString filter() const;

    QStringList collection() const;
    void setCollection(const QStringList &collection);
    QString currentValue() const;

private:
    QString m_caption;
    QString m_windowCaption;
    QString m_dir;
    QString m_filter;
    QStringList m_collection;
};

#endif // OPERATORPARAMETERFILESCOLLECTION_H
