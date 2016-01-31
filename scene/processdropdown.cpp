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
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPainter>

#include "processdropdown.h"
#include "process.h"
#include "processnode.h"
#include "operatorparameterdropdown.h"
#include "preferences.h"

ProcessDropDown::ProcessDropDown(QRectF rect,
                                 OperatorParameterDropDown *dropdown,
                                 Process *process,
                                 ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_dropdown(dropdown),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false)
{
    setPen(QPen(preferences->color(QPalette::Window), PEN_WIDTH));
    setBrush(QBrush(preferences->color(QPalette::Base)));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2+MARGIN, rect.y()+MARGIN, rect.width()/2-MARGIN*2, rect.height()-MARGIN*2);
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(dropdown->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());
    m_caption->setDefaultTextColor(preferences->color(QPalette::WindowText));

    m_currentValue= new QGraphicsTextItem(dropdown->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());
    m_currentValue->setDefaultTextColor(preferences->color(QPalette::ButtonText));
    connect(dropdown, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged(QString)));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

}

ProcessDropDown::~ProcessDropDown()
{

}

void ProcessDropDown::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    if (m_mouseHover)
        painter->setPen(preferences->color(QPalette::Highlight));
    else
        painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(path());

}

int ProcessDropDown::type() const
{
    return Type;
}

void ProcessDropDown::clicked(QPoint pos)
{
    m_dropdown->dropDown(pos);
}

void ProcessDropDown::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=true;
    update();
}

void ProcessDropDown::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}

void ProcessDropDown::valueChanged(const QString &value)
{
    m_currentValue->setPlainText(value);
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}
