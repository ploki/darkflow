#include <QJsonArray>

#include "operatorparameterfilescollection.h"

OperatorParameterFilesCollection::OperatorParameterFilesCollection(
        const QString& name,
        const QString& caption,
        const QString& windowCaption,
        const QString& dir,
        const QString& filter,
        QObject *parent) :
    OperatorParameter(name, caption, parent),
    m_windowCaption(windowCaption),
    m_dir(dir),
    m_filter(filter),
    m_collection()
{

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

QJsonObject OperatorParameterFilesCollection::save()
{
    QJsonObject obj;
    QJsonArray files;
    obj["type"] = "filesCollection";
    obj["name"] = m_name;
    foreach(const QString& file, m_collection) {
        qWarning("saving a file");
        files.push_back(file);
    }
    obj["files"] = files;
    return obj;
}

void OperatorParameterFilesCollection::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "filesCollection" ) {
        qWarning("invalid parameter type");
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        qWarning("invalid parameter name");
        return;
    }
    QJsonArray files = obj["files"].toArray();
    foreach(QJsonValue val, files) {
        m_collection.push_back(val.toString());
    }
    emit updated();
}



