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

#include "process.h"
#include "processnode.h"
#include "processslider.h"
#include "slider.h"
#include "operatorparameterslider.h"
#include "operator.h"
#include "preferences.h"

ProcessSlider::ProcessSlider(QRectF rect,
                             OperatorParameterSlider *slider,
                             Process *process,
                             ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_slider(slider),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false),
    m_sliderDialog(new Slider(m_slider->windowCaption(),m_slider->unit(),
                              m_slider->scale(),m_slider->numberSet(),
                              m_slider->getMin(), m_slider->getMax(),
                              m_slider->value(),m_slider->hardMin(),
                              m_slider->hardMax(),m_slider->parametersFilter(),
                              NULL))
{
    setPen(QPen(preferences->color(QPalette::Window), PEN_WIDTH));
    setBrush(QBrush(preferences->color(QPalette::Base)));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2+MARGIN, rect.y()+MARGIN, rect.width()/2-MARGIN*2, rect.height()-MARGIN*2);
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(m_slider->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());
    m_caption->setDefaultTextColor(preferences->color(QPalette::WindowText));

    m_currentValue= new QGraphicsTextItem(m_slider->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());
    m_currentValue->setDefaultTextColor(preferences->color(QPalette::ButtonText));

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    connect(m_sliderDialog, SIGNAL(updated()), this, SLOT(updateValue()));
    connect(m_slider, SIGNAL(updated()), this, SLOT(updateLoad()));
    updateValue();

}

ProcessSlider::~ProcessSlider()
{
    delete m_sliderDialog;
}

void ProcessSlider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    if (m_mouseHover)
        painter->setPen(QPen(preferences->color(QPalette::Highlight)));
    else
        painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(path());

}

int ProcessSlider::type() const
{
    return Type;
}

void ProcessSlider::clicked(QPoint pos)
{
    if ( !m_sliderDialog->isVisible() ) {
        QPoint newPos = pos;
        newPos.setX(newPos.x() - m_sliderDialog->size().width()/2);
        newPos.setY(newPos.y() - m_sliderDialog->size().height()/2);
        m_sliderDialog->move(newPos.x(), newPos.y());
        m_sliderDialog->show();
    }
    m_sliderDialog->activateWindow();
    m_sliderDialog->raise();
}

void ProcessSlider::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=true;
    update();
}

void ProcessSlider::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}

void ProcessSlider::apply()
{
    m_node->m_operator->play();
}

void ProcessSlider::updateValue()
{
    m_slider->setUnit(m_sliderDialog->getUnit());
    m_slider->setScale(m_sliderDialog->getScale());
    m_slider->setMin(m_sliderDialog->getMin());
    m_slider->setMax(m_sliderDialog->getMax());
    m_slider->setValue(m_sliderDialog->getValue());

    m_currentValue->setPlainText(m_sliderDialog->currentValue());
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}

void ProcessSlider::updateLoad()
{
    m_sliderDialog->loadValues(m_slider->unit(),
                               m_slider->scale(),
                               m_slider->getMin(),
                               m_slider->getMax(),
                               m_slider->value());
}
