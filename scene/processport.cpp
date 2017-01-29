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
#include <QString>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPolygon>
#include <QVector>
#include <QPoint>
#include <QGraphicsTextItem>
#include <QPainter>

#include "processport.h"
#include "process.h"
#include "processnode.h"
#include "preferences.h"

ProcessPort::ProcessPort(QRectF rect,
                         const QString &portName,
                         int portIdx,
                         ProcessPort::PortType portType,
                         Process *process, ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_node(node),
    m_process(process),
    m_portName(portName),
    m_portType(portType),
    m_portIdx(portIdx)
{
    const qreal flange=2.;
    QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
    textItem->setFont(preferences->getWorkspaceFont());
    textItem->setDefaultTextColor(preferences->color(QPalette::WindowText));
    textItem->setPlainText(m_portName);
    qreal textW = textItem->boundingRect().width();
    qreal textH = textItem->boundingRect().height();

    qreal unit = textH/4;
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = rect.width();
    qreal h = rect.height();
    y+= (h-unit*2)/2;
    setPen(QPen(preferences->color(QPalette::Window), PEN_WIDTH));
    setBrush(QBrush(preferences->color(QPalette::Highlight)));

    QVector<QPoint> points;

    switch (portType) {
    case ProcessPort::InputPort:
    case ProcessPort::InputOnePort:
        x-=flange;
        textItem->setPos(x+unit*4,y-unit);
        points.push_back(QPoint(x+0,y+0));
        points.push_back(QPoint(x+unit*2,y+0));
        points.push_back(QPoint(x+unit*4,y+unit));
        points.push_back(QPoint(x+unit*2,y+unit*2));
        points.push_back(QPoint(x+0,y+unit*2));
        points.push_back(QPoint(x+0,y+0));
        break;
    case ProcessPort::OutputPort:
        x+=flange+w;
        textItem->setPos(x-unit*4-textW,y-unit);
        points.push_back(QPoint(x-0,y+0));
        points.push_back(QPoint(x-unit*2,y+0));
        points.push_back(QPoint(x-unit*4,y+unit));
        points.push_back(QPoint(x-unit*2,y+unit*2));
        points.push_back(QPoint(x-0,y+unit*2));
        points.push_back(QPoint(x-0,y+0));
    }


    QPolygon poly(points);
    QPainterPath pp;
    pp.addPolygon(poly);
    setPath(pp);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

ProcessPort::~ProcessPort()
{
}

void ProcessPort::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path());
}

int ProcessPort::type() const
{
    return ProcessPort::Type;
}

QVariant ProcessPort::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged)
    {
        emit positionChanged();
    }
    return value;
}

QPointF ProcessPort::anchorScenePos()
{
    QPointF anchor = boundingRect().center();
    return mapToScene(anchor);

}

ProcessPort::PortType ProcessPort::portType() const
{
    return m_portType;
}
int ProcessPort::portIdx() const
{
    return m_portIdx;
}

