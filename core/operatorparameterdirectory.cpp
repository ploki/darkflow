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
#include "operatorparameterdirectory.h"
#include "console.h"

#include <QDir>
#include <QFileDialog>
#include "process.h"
#include <QApplication>

OperatorParameterDirectory::OperatorParameterDirectory(const QString &name,
                                                       const QString &caption,
                                                       const QString &baseDir,
                                                       const QString &currentValue,
                                                       Operator *op) :
    OperatorParameter(name, caption, op),
    m_baseDir(baseDir),
    m_currentValue(currentValue)
{

}

OperatorParameterDirectory::~OperatorParameterDirectory()
{
}

void OperatorParameterDirectory::askForDirectory()
{
    QWidget *activeWindow = qApp->activeWindow();
    QString newValue = QFileDialog::getExistingDirectory(activeWindow,
                                                         m_caption,
                                                         m_currentValue, 0);
    if ( !newValue.isEmpty() && m_currentValue != newValue ) {
        m_currentValue = newValue;
        emit updated();
    }
}

QString OperatorParameterDirectory::currentValue() const
{
    return m_currentValue;
}

QJsonObject OperatorParameterDirectory::save(const QString &baseDirStr)
{
    QDir baseDir(baseDirStr);
    QJsonObject obj;
    obj["type"] = QString("directory");
    obj["name"] = m_name;
    obj["target"] = baseDir.relativeFilePath(m_currentValue);
    return obj;
}

void OperatorParameterDirectory::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "directory" ) {
        dflWarning(tr("Directory: invalid parameter type"));
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        dflWarning(tr("Directory: invalid parameter name"));
        return;
    }
    QDir baseDir(m_baseDir);
    QString relativeDir = obj["target"].toString();
    m_currentValue = baseDir.absoluteFilePath(relativeDir);
    emit updated();
}
