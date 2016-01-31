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

#include "processfilescollection.h"
#include "process.h"
#include "processnode.h"
#include "operatorparameterfilescollection.h"
#include "filesselection.h"
#include "preferences.h"

ProcessFilesCollection::ProcessFilesCollection(QRectF rect,
                                 OperatorParameterFilesCollection *filesCollection,
                                 Process *process,
                                 ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_filesCollection(filesCollection),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false),
    m_selectionDialog(new FilesSelection(filesCollection->windowCaption(),
                                         filesCollection->dir(),
                                         filesCollection->filter(),
                                         NULL))
{
    setPen(QPen(preferences->color(QPalette::Window), PEN_WIDTH));
    setBrush(QBrush(preferences->color(QPalette::Base)));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2+MARGIN, rect.y()+MARGIN, rect.width()/2-MARGIN*2, rect.height()-MARGIN*2);
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(filesCollection->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());
    m_caption->setDefaultTextColor(preferences->color(QPalette::WindowText));

    m_currentValue= new QGraphicsTextItem(filesCollection->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());
    m_currentValue->setDefaultTextColor(preferences->color(QPalette::ButtonText));
    //connect(dropdown, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged(QString)));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    connect(m_selectionDialog, SIGNAL(accepted()), this, SLOT(selectionAccepted()));
    connect(m_selectionDialog, SIGNAL(rejected()), this, SLOT(selectionRejected()));
    connect(m_filesCollection, SIGNAL(updated()), this, SLOT(updateValue()));
}

ProcessFilesCollection::~ProcessFilesCollection()
{
    delete m_selectionDialog;
}

void ProcessFilesCollection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

int ProcessFilesCollection::type() const
{
    return Type;
}

void ProcessFilesCollection::clicked(QPoint pos)
{
    QPoint newPos = pos;
    newPos.setX(newPos.x() - m_selectionDialog->size().width()/2);
    newPos.setY(newPos.y() - m_selectionDialog->size().height()/2);
    m_selectionDialog->move(newPos.x(), newPos.y());
    m_selectionDialog->setSelection(m_filesCollection->collection());
    m_selectionDialog->show();
}

void ProcessFilesCollection::selectionAccepted()
{
    m_filesCollection->setCollection(m_selectionDialog->getSelection());
    m_selectionDialog->clearSelection();
    updateValue();
}

void ProcessFilesCollection::selectionRejected()
{
    m_selectionDialog->clearSelection();
}


void ProcessFilesCollection::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=true;
    update();
}

void ProcessFilesCollection::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}

void ProcessFilesCollection::updateValue()
{
    m_currentValue->setPlainText(m_filesCollection->currentValue());
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}
