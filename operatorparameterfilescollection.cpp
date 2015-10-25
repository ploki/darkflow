#include "operatorparameterfilescollection.h"

OperatorParameterFilesCollection::OperatorParameterFilesCollection(
        QString caption,
        QString windowCaption,
        QString dir,
        QString filter,
        QObject *parent) :
    OperatorParameter(parent),
    m_caption(caption),
    m_windowCaption(windowCaption),
    m_dir(dir),
    m_filter(filter),
    m_collection()
{

}

QString OperatorParameterFilesCollection::caption() const
{
    return m_caption;
}

QString OperatorParameterFilesCollection::windowCaption() const
{
    return m_windowCaption;
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

QString OperatorParameterFilesCollection::currentValue() const
{
    int count = m_collection.count();
    if ( count > 1)
        return QString("%0 files").arg(count);
    else
        return QString("%0 file").arg(count);
}



