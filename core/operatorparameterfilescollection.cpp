/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include <QJsonArray>

#include "operatorparameterfilescollection.h"
#include "console.h"
#include <QDir>

OperatorParameterFilesCollection::OperatorParameterFilesCollection(
        const QString& name,
        const QString& caption,
        const QString& windowCaption,
        const QString& dir,
        const QString& filter,
        Operator *op) :
    OperatorParameter(name, caption, op),
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
    emit setOutOfDate();
}

QString OperatorParameterFilesCollection::currentValue() const
{
    int count = m_collection.count();
    return tr("%n file(s)", "", count);
}

QJsonObject OperatorParameterFilesCollection::save(const QString& baseDirStr)
{
    QDir baseDir(baseDirStr);
    QJsonObject obj;
    QJsonArray files;
    obj["type"] = QString("filesCollection");
    obj["name"] = m_name;
    foreach(QString file, m_collection) {
        dflDebug(tr("FilesCollection: saving a file"));
        file = baseDir.relativeFilePath(file);
        files.push_back(file);
    }
    obj["files"] = files;
    return obj;
}

void OperatorParameterFilesCollection::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "filesCollection" ) {
        dflWarning(tr("FilesCollection: invalid parameter type"));
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        dflWarning(tr("FilesCollection: invalid parameter name"));
        return;
    }
    QJsonArray files = obj["files"].toArray();
    foreach(QJsonValue val, files) {
        QString file = val.toString();
        QDir baseDir(m_dir);
        file = baseDir.absoluteFilePath(file);
        m_collection.push_back(file);
    }
    emit updated();
}



