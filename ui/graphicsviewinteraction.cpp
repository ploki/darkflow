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
#include "graphicsviewinteraction.h"
#include "processconnection.h"

#include <QGraphicsView>
#include <QGestureEvent>
#include <QGraphicsItem>
#include <QApplication>

GraphicsViewInteraction::GraphicsViewInteraction(QGraphicsView *graphicsView, QObject *parent)
    : QObject(parent),
      m_graphicsView(graphicsView),
      totalScaleFactor(1),
      lastGestureFactor(1)
{
    parent->installEventFilter(this);
    graphicsView->installEventFilter(this);
    /* viewport receives first wheel events, it must ignore it to prevent scrolls */
    graphicsView->viewport()->installEventFilter(this);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    /* multitouch related stuffs */
    m_graphicsView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    m_graphicsView->grabGesture(Qt::PinchGesture);
}

bool GraphicsViewInteraction::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {

    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        bool doScale = false;
        qreal scaleFactor = 1;
        int key = dynamic_cast<QKeyEvent*>(event)->key();
        switch (key) {
        case Qt::Key_1:
            scaleFactor = 1; doScale = true;
            break;
        case Qt::Key_2:
            scaleFactor = 2; doScale = true;
            break;
        case Qt::Key_3:
            scaleFactor = .5; doScale = true;
            break;
        case Qt::Key_F: {
            fitVisible();
            emit zoomChanged(-1);
            event->accept();
            return true;
        }
        }
        if (doScale) {
            totalScaleFactor = lastGestureFactor = 1;
            zoomUpdate(scaleFactor);
            event->accept();
            return true;
        }
        break;
    }
    case QEvent::Wheel:
    {
        if ( !(QApplication::keyboardModifiers() & Qt::ControlModifier) )
            break;
        int delta = dynamic_cast<QWheelEvent*>(event)->delta();
        if(delta > 0)
            totalScaleFactor *= (1.+delta/400.);
        else
            totalScaleFactor /= (1.-delta/400.);
        lastGestureFactor = 1;
        zoomUpdate(totalScaleFactor);
        event->accept();
        return true;
    }
    case QEvent::Gesture:
    {
        QGestureEvent* gestureEvent = static_cast<QGestureEvent*>(event);
        if (QGesture *pinch = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture* pinchGesture = static_cast<QPinchGesture *>(pinch);
            QPinchGesture::ChangeFlags changeFlags = pinchGesture->changeFlags();
            if (changeFlags & QPinchGesture::ScaleFactorChanged) {
                qreal currentScaleFactor = pinchGesture->totalScaleFactor()/2.+.5;
                //almost fixes glitches and resets in pinch
                if (currentScaleFactor >= 0.95 && currentScaleFactor <= 1.053 &&
                        (lastGestureFactor < 0.85 || lastGestureFactor > 1.176) )
                    totalScaleFactor *= lastGestureFactor;
                lastGestureFactor = currentScaleFactor;
                zoomUpdate(totalScaleFactor * currentScaleFactor);
                if (pinch->state() == Qt::GestureFinished) {
                    totalScaleFactor *= currentScaleFactor;
                    lastGestureFactor = 1;
                }
            }
            event->accept();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return parent()->eventFilter(obj, event);

}


void GraphicsViewInteraction::zoomApply(qreal factor)
{
    m_graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    m_graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_graphicsView->setTransform(QTransform().scale(factor, factor));
}
void GraphicsViewInteraction::zoomUpdate(qreal factor)
{
    zoomApply(factor);
    emit zoomChanged(factor);
}

void GraphicsViewInteraction::fitVisible()
{
    QRectF united;
    bool first = true;
    foreach(QGraphicsItem *item, m_graphicsView->scene()->items()) {
        if (item->type() == ProcessConnection::Type ) {
            continue;
        }

        if (first) {
            first = false;
            united = item->sceneBoundingRect();
        }
        else {
            united = united.united(item->sceneBoundingRect());
        }
        if ( item->type() == QGraphicsPixmapItem::Type ) {
            united = item->sceneBoundingRect();
            break;
        }
    }
    QRectF before = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect()).boundingRect();
    m_graphicsView->fitInView(united,Qt::KeepAspectRatio);
    QRectF after = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect()).boundingRect();
    totalScaleFactor *= before.width()/after.width();
}

void GraphicsViewInteraction::zoomSet(qreal factor)
{
    lastGestureFactor = 1;
    totalScaleFactor = factor;
    zoomApply(factor);
}

void GraphicsViewInteraction::zoomIn()
{
    zoomSet(totalScaleFactor*1.1);
}

void GraphicsViewInteraction::zoomOut()
{
    zoomSet(totalScaleFactor/1.1);
}
