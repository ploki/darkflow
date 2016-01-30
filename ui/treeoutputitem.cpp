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
#include "treeoutputitem.h"
#include "operator.h"
#include "operatoroutput.h"
#include <QObject>

TreeOutputItem::TreeOutputItem(OperatorOutput *output,
                               int idx,
                               Role role,
                               QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_output(output),
    m_idx(idx),
    m_role(role),
    m_caption()
{
    if ( Source == role )
        m_caption = m_output->m_operator->getName() + ": " + m_output->name();
    else
        m_caption = m_output->name();
    setCaption();
}

void TreeOutputItem::setCaption()
{
    setFont(0, QFont("Sans", 12));
    if ( Source == m_role ) {
        setBackground(0, QBrush(Qt::yellow));
        setText(0, m_caption);
    }
    else if ( EnabledSink == m_role ) {
        setBackground(0, QBrush(Qt::yellow));
        setText(0, tr("☑ %0").arg(m_caption));
    }
    else {
        setBackground(0, QBrush(Qt::gray));
        setText(0, tr("☐ %0").arg(m_caption));
    }
}

void TreeOutputItem::setRole(const Role &role)
{
    m_role = role;
}

TreeOutputItem::~TreeOutputItem()
{
}

OperatorOutput *TreeOutputItem::output() const
{
    return m_output;
}

int TreeOutputItem::idx() const
{
    return m_idx;
}

TreeOutputItem::Role TreeOutputItem::role() const
{
    return m_role;
}

