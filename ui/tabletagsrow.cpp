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
#include "process.h"
#include "tabletagsrow.h"
#include <QTableWidget>
#include "tablewidgetitem.h"
#include "operator.h"
#include "preferences.h"

TableTagsRow::TableTagsRow(const QString& photoIdentity,
                           const QString &key,
                           const QString &value,
                           TableTagsRow::Source source,
                           QTableWidget *tableWidget,
                           Operator *op) :
    QObject(NULL),
    m_photoIdentity(photoIdentity),
    m_key(new TableWidgetItem(key, TableWidgetItem::Key, this)),
    m_previousKey(key),
    m_value(new TableWidgetItem(value, TableWidgetItem::Value, this)),
    m_originalValue(value),
    m_table(tableWidget),
    m_source(source),
    m_dirty(false),
    m_operator(op),
    m_initDone(false)
{
    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow(rowCount);
    tableWidget->setItem(rowCount, 0, m_key);
    tableWidget->setItem(rowCount, 1, m_value);
    if ( m_source == FromPhoto )
        m_key->setFlags(Qt::NoItemFlags);
    m_initDone=true;
    setColors();
}

TableTagsRow::~TableTagsRow()
{
    m_table->removeRow(m_key->row());
}

void TableTagsRow::reset()
{
    if ( m_source == FromPhoto ) {
        m_value->setText(m_originalValue);
        m_dirty = false;
        m_operator->resetTagOverride(m_photoIdentity,
                                     m_key->text());
        setColors();
    }
}

bool TableTagsRow::remove()
{
    bool ret = false;
    switch(m_source) {
    case FromPhoto:
        setValue("");
        ret = true;
        break;
    case FromOperator:
        ret = false;
        m_operator->resetTagOverride(m_photoIdentity, m_key->text());
        break;
    }
    return ret;
}

void TableTagsRow::setValue(const QString& value, bool initial)
{
    m_value->setText(value);
    if ( m_source == FromPhoto ) {
        m_dirty = true;
    }
    setColors();
    if ( !initial )
        changed(TableWidgetItem::Value);
}

QString TableTagsRow::getKey() const
{
    return m_key->text();
}

void TableTagsRow::setColors()
{
    QColor color;
    switch(m_source) {
    case FromPhoto:
        if ( m_dirty )
            color = Qt::darkRed;
        else
            color = preferences->color(QPalette::Text);
        break;
    case FromOperator:
        color = Qt::darkGreen;
        break;
    }
    m_key->setTextColor(color);
    m_value->setTextColor(color);

}

void TableTagsRow::changed(TableWidgetItem::FieldType fieldType)
{
    if (!m_initDone) return;
    if ( fieldType == TableWidgetItem::Key &&
         m_source == FromOperator ) {
        m_operator->resetTagOverride(m_photoIdentity, m_previousKey);
        m_previousKey = m_key->text();
    }
    m_operator->setTagOverride(m_photoIdentity,
                               m_key->text(),
                               m_value->text());
    if (m_source == FromPhoto)
        m_dirty = true;
    setColors();
}
