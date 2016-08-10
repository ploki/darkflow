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
#ifndef OPERATORPARAMETERDROPDOWN_H
#define OPERATORPARAMETERDROPDOWN_H

#include <QMap>
#include <QString>
#include <QPoint>

#include "operatorparameter.h"

#define DF_TR_AND_C(str) (str), tr(str)

class QObject;
class QMenu;
class QAction;

class DropDownPair {
public:
    DropDownPair();
    DropDownPair(const char *str, int v);
    QString C_option;
    int value;
};

class OperatorParameterDropDown : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterDropDown(
            const QString& name,
            const QString& caption,
            Operator *op,
            const char *slot);
    ~OperatorParameterDropDown();

    void addOption(const char *option, const QString& optionLocalized, int value, bool selected=false);
    void dropDown(const QPoint& pos);

    QString currentValue() const;

    QJsonObject save(const QString& baseDirStr);
    void load(const QJsonObject &obj);

signals:
    /* used to notify ProcessDropDown */
    void valueChanged(const QString& value);

    /* used to notify Operator */
    void itemSelected(int v);

private slots:
    /* called by QMenu */
    void actionTriggered(QAction *action);

private:
    QMenu *m_menu;
    QMap<QString, DropDownPair> m_options;
    QString m_currentValue;
};

#endif // OPERATORPARAMETERDROPDOWN_H
