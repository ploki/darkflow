#ifndef OPERATORPARAMETERFILESCOLLECTION_H
#define OPERATORPARAMETERFILESCOLLECTION_H

#include <QString>
#include <QStringList>
#include "operatorparameter.h"

class OperatorParameterFilesCollection : public OperatorParameter
{
public:
    OperatorParameterFilesCollection(QString caption,
                                     QString dir,
                                     QString filter,
                                     QObject *parent = 0);
    QString caption() const;
    void setCaption(const QString &caption);

    QString dir() const;
    void setDir(const QString &dir);

    QString filter() const;
    void setFilter(const QString &filter);

    QStringList collection() const;
    void setCollection(const QStringList &collection);

private:
    QString m_caption;
    QString m_dir;
    QString m_filter;
    QStringList m_collection;
};

#endif // OPERATORPARAMETERFILESCOLLECTION_H
