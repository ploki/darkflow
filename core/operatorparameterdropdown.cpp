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
#include <QMenu>
#include "operatorparameterdropdown.h"
#include "console.h"

OperatorParameterDropDown::OperatorParameterDropDown(
        const QString& name,
        const QString& caption,
        Operator *op,
        const char *slot) :
    OperatorParameter(name, caption, op),
    m_menu(new QMenu),
    m_currentValue()
{
    connect(this, SIGNAL(itemSelected(int)), op,slot);
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
}

OperatorParameterDropDown::~OperatorParameterDropDown()
{
    delete m_menu;
}

void OperatorParameterDropDown::addOption(const char * option,
                                          const QString &optionLocalized,
                                          int value,
                                          bool selected)
{
    m_options[optionLocalized] = DropDownPair(option, value);
    m_menu->addAction(QIcon(), optionLocalized);
    if (selected)
        m_currentValue = optionLocalized;
}

void OperatorParameterDropDown::dropDown(const QPoint& pos)
{
    m_menu->exec(pos);
}

void OperatorParameterDropDown::actionTriggered(QAction *action)
{
    m_currentValue=action->text();
    emit itemSelected(m_options[m_currentValue].value);
    emit valueChanged(m_currentValue);
}

QString OperatorParameterDropDown::currentValue() const
{
    return m_currentValue;
}

QJsonObject OperatorParameterDropDown::save(const QString&)
{
    QJsonObject obj;
    obj["type"] = QString("dropdown");
    obj["name"] = m_name;
    obj["currentValue"] = m_options[m_currentValue].C_option;
    return obj;
}

void OperatorParameterDropDown::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "dropdown" ) {
        dflWarning(tr("DropDown: invalid parameter type"));
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        dflWarning(tr("DropDown: invalid parameter name"));
        return;
    }
    bool actionFound = false;
    QString currentValue = obj["currentValue"].toString();
    for ( QMap<QString, DropDownPair>::iterator pair = m_options.begin();
          pair != m_options.end();
          ++pair) {
        if ( currentValue == pair.value().C_option ) {
            currentValue = pair.key();
            actionFound = true;
            break;
        }
    }
    if ( !actionFound ) {
        dflWarning(tr("DropDown: could not find localized value"));
        return;
    }
    actionFound = false;
    foreach(QAction *action, m_menu->actions()) {
        if ( action->text() == currentValue ) {
            actionFound = true;
            action->trigger();
            break;
        }
    }
    if (!actionFound)
        dflWarning(tr("DropDown: unknown value for dropdown"));
}



DropDownPair::DropDownPair() : C_option(), value(-1)
{
}

DropDownPair::DropDownPair(const char *str, int v) :
    C_option(str),
    value(v)
{
}
