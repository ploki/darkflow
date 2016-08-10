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
#include <QPainterPath>
#include <QPainter>
#include <QString>
#include <QGraphicsTextItem>
#include <QEvent>
#include <QCursor>
#include "processbutton.h"
#include "process.h"
#include "processnode.h"
#include "preferences.h"

static QString buttonText(ProcessButton::ButtonType type)
{
    switch(type) {
#ifdef Q_OS_MAC
    case ProcessButton::Play: return ProcessButton::tr("⏯");
    case ProcessButton::Abort:return ProcessButton::tr("⏹");
    case ProcessButton::Display: return ProcessButton::tr("📝");
    case ProcessButton::Close: return ProcessButton::tr("❌");
    case ProcessButton::Help: return ProcessButton::tr("❓");
    case ProcessButton::Refresh: return ProcessButton::tr("🔄");
#else
    case ProcessButton::Play: return ProcessButton::tr("▶");
    case ProcessButton::Abort:return ProcessButton::tr("■");
    case ProcessButton::Display: return ProcessButton::tr("☷");
    case ProcessButton::Close: return ProcessButton::tr("❌");
    case ProcessButton::Help: return ProcessButton::tr("?");
    case ProcessButton::Refresh: return ProcessButton::tr("⟳");
#endif
    default:
        return ProcessButton::tr("0");
    }
}

ProcessButton::ProcessButton(QRectF rect,
                             ProcessButton::ButtonType type,
                             Process *process,
                             ProcessNode *node) :
    QObject(NULL),
    QGraphicsRectItem(QRectF(rect.x()+PEN_WIDTH+MARGIN,rect.y()+PEN_WIDTH+MARGIN,
                             rect.width()-PEN_WIDTH*2-MARGIN*2,rect.height()-PEN_WIDTH*2-MARGIN*2),node),
    m_process(process),
    m_node(node),
    m_type(type),
    m_mouseHover(false),
    m_mousePress(false)
{
    //qreal radius=3;

    setPen(QPen(preferences->color(QPalette::Window),PEN_WIDTH));
    setBrush(QBrush(preferences->color(QPalette::Base)));

    QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(buttonText(m_type));
    textItem->setDefaultTextColor(preferences->color(QPalette::ButtonText));
    QPointF textCenter = textItem->boundingRect().center();
    QPointF boxCenter = boundingRect().center();
    textItem->setPos(boxCenter-textCenter);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
}

ProcessButton::~ProcessButton()
{

}

void ProcessButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_mouseHover)
        painter->setPen(preferences->color(QPalette::Highlight));
    else
        painter->setPen(pen());
    if (m_mousePress)
        painter->setBrush(preferences->color(QPalette::Button));
    else
        painter->setBrush(brush());
//        painter->setBrush(Qt::transparent);
    painter->setRenderHint(QPainter::Antialiasing);
    //painter->drawPath(path());
    painter->drawRect(boundingRect());
}


int ProcessButton::type() const
{
    return ProcessButton::Type;
}

void ProcessButton::resetMouse()
{
    m_mousePress=false;
    update();
}

void ProcessButton::mousePress()
{
    m_mousePress=true;
}

void ProcessButton::mouseRelease(QPoint screenPos)
{
    if ( m_mousePress )
    {
        emit buttonClicked(screenPos);
    }
    m_mousePress=false;
}

void ProcessButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
   Q_UNUSED(event);
   m_mouseHover=true;
   update();
}

void ProcessButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}
