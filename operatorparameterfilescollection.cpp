#include "operatorparameterfilescollection.h"

OperatorParameterFilesCollection::OperatorParameterFilesCollection(
        QString caption,
        QString dir,
        QString filter,
        QObject *parent) :
    OperatorParameter(parent),
    m_caption(caption),
    m_dir(dir),
    m_filter(filter),
    m_collection()
{

}

QString OperatorParameterFilesCollection::caption() const
{
    return m_caption;
}

QString OperatorParameterFilesCollection::dir() const
{
    return m_dir;
}

QString OperatorParameterFilesCollection::filter() const
{
    return m_filter;
}

QStringList OperatorParameterFilesCollection::collection() const
{
    return m_collection;
}

void OperatorParameterFilesCollection::setCollection(const QStringList &collection)
{
    m_collection=collection;
    emit parameterChanged();
}



