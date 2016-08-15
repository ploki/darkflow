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
#ifndef TABLETAGSROW_H
#define TABLETAGSROW_H

#include <QObject>
#include "tablewidgetitem.h"

class QTableWidget;
class TableWidgetItem;
class Operator;

class TableTagsRow : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        FromPhoto,
        FromOperator,
    } Source;
    explicit TableTagsRow(const QString& photoIdentity,
                          const QString& key,
                          const QString& value,
                          Source source,
                          QTableWidget *tableWidget,
                          Operator *op);
    ~TableTagsRow();

    void reset();
    bool remove();
    void setValue(const QString& value, bool initial = false);
    QString getKey() const;

private:
    QString m_photoIdentity;
    TableWidgetItem *m_key;
    QString m_previousKey;
    TableWidgetItem *m_value;
    QString m_originalValue;
    QTableWidget *m_table;
    Source m_source;
    bool m_dirty;
    Operator *m_operator;
    bool m_initDone;

private slots:
    friend class TableWidgetItem;
    void setColors();
    void changed(TableWidgetItem::FieldType fieldType);
};

#endif // TABLETAGSROW_H
