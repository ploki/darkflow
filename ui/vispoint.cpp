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
#include "vispoint.h"
#include <QPainterPath>
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <visualization.h>

VisPoint::VisPoint(QPointF pos,
                   Visualization *vis,
                   int number,
                   QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    m_vis(vis),
    m_number(number)
{
    qreal x = pos.x();
    qreal y = pos.y();
    QPainterPath pp;
    pp.addRoundedRect(x-3,y-3,6.,6.,1.,1.);
    pp.moveTo(x-10,y);
    pp.lineTo(x+10,y);
    pp.moveTo(x,y-10);
    pp.lineTo(x,y+10);
    pp.addText(x+7,y-7, QFont("Sans",5,5,false), QString::number(number,10));
    setPath(pp);
    setPen(QPen(Qt::green, 1));
    setFlag(QGraphicsItem::ItemIsSelectable);
    setCursor(QCursor(Qt::ArrowCursor));
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void VisPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path());
}

QVariant VisPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if ( change == ItemScenePositionHasChanged )
        m_vis->storePoints();
    return value;
}

int VisPoint::type() const
{
    return Type;
}

QPointF VisPoint::position() const
{
    QPointF pos(boundingRect().x()+10.5, boundingRect().y()+boundingRect().height()-10.5);
    return mapToScene(pos);
}
