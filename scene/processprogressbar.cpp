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
#include <QPainter>

#include "processprogressbar.h"
#include "process.h"
#include "processnode.h"
#include "preferences.h"

ProcessProgressBar::ProcessProgressBar(QRectF rect, Process *process, ProcessNode *node) :
    QObject(NULL),
    QGraphicsRectItem(QRectF(rect.x()+PEN_WIDTH+MARGIN*2,rect.y()+PEN_WIDTH+MARGIN*2,
                             rect.width()-PEN_WIDTH*2-MARGIN*4,rect.height()-PEN_WIDTH*2-MARGIN*4),node),
    m_rect(rect.x()+PEN_WIDTH+MARGIN*2,rect.y()+PEN_WIDTH+MARGIN*2,
           rect.width()-PEN_WIDTH*2-MARGIN*4,rect.height()-PEN_WIDTH*2-MARGIN*4),
    m_overlay(NULL),
    m_process(process),
    m_node(node),
    m_progress(0),
    m_complete(1)
{
    m_overlay = new QGraphicsRectItem(m_rect,this);
    m_overlay->setBrush(preferences->color(QPalette::Highlight));
    m_overlay->setPen(QPen(preferences->color(QPalette::Base),PEN_WIDTH));
    setBrush(preferences->color(QPalette::Base));
    setPen(QPen(preferences->color(QPalette::Window), PEN_WIDTH));
    setFlags(QGraphicsItem::ItemIsSelectable);
}

ProcessProgressBar::~ProcessProgressBar()
{

}

void ProcessProgressBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRect(m_rect);
    m_overlay->setRect(QRect(m_rect.x(),m_rect.y(),
                             m_rect.width()*double(m_progress)/double(m_complete) ,m_rect.height()));
}

int ProcessProgressBar::type() const
{
    return Type;
}

void ProcessProgressBar::progress(int p, int c)
{
    m_progress = p;
    m_complete = c;
    update();
}
