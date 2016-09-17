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
#include <QGraphicsPathItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPainter>

#include "processdirectory.h"
#include "process.h"
#include "processnode.h"
#include "operatorparameterdirectory.h"
#include "preferences.h"


ProcessDirectory::ProcessDirectory(QRectF rect,
                                   OperatorParameterDirectory *directory,
                                   Process *process,
                                   ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_directory(directory),
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

    m_caption = new QGraphicsTextItem(directory->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());
    m_caption->setDefaultTextColor(preferences->color(QPalette::WindowText));

    m_currentValue= new QGraphicsTextItem(tr("..."),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());
    m_currentValue->setDefaultTextColor(preferences->color(QPalette::ButtonText));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

}

ProcessDirectory::~ProcessDirectory()
{

}

void ProcessDirectory::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

int ProcessDirectory::type() const
{
    return Type;
}

void ProcessDirectory::clicked(QPoint)
{
    m_directory->askForDirectory();
}

void ProcessDirectory::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=true;
    update();
}

void ProcessDirectory::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}
