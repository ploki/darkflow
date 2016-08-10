#include "graphicsviewinteraction.h"
#include "processconnection.h"

#include <QGraphicsView>
#include <QGestureEvent>
#include <QGraphicsItem>

GraphicsViewInteraction::GraphicsViewInteraction(QGraphicsView *graphicsView, QObject *parent)
    : QObject(parent),
      m_graphicsView(graphicsView),
      totalScaleFactor(1),
      lastGestureFactor(1),
      zoomKeyPressed(false)
{
    parent->installEventFilter(this);
    graphicsView->installEventFilter(this);
    /* viewport receives first wheel events, it must ignore it to prevent scrolls */
    graphicsView->viewport()->installEventFilter(this);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    /* multitouch related stuffs */
    m_graphicsView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    m_graphicsView->grabGesture(Qt::PinchGesture);
    m_graphicsView->grabGesture(Qt::PanGesture);
}

bool GraphicsViewInteraction::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        int key = dynamic_cast<QKeyEvent*>(event)->key();
        switch (key) {
        case Qt::Key_Control: {
            zoomKeyPressed = event->type() == QEvent::KeyPress;
            event->accept();
            return true;
        }
        case Qt::Key_1: {
            totalScaleFactor = lastGestureFactor = 1;
            zoomUpdate(1);
            event->accept();
            return true;
        }
        case Qt::Key_F: {
            fitVisible();
            emit zoomChanged(-1);
            event->accept();
            return true;
        }
        }
        break;
    }
    case QEvent::Wheel:
    {
        if (!zoomKeyPressed)
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
        if (QGesture *pan = gestureEvent->gesture(Qt::PanGesture)) {
            QPanGesture *panGesture = static_cast<QPanGesture *>(pan);
            QPointF delta = panGesture->delta();
            if (!delta.isNull()){
                QPointF center = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect().center());
                center -= delta;
                m_graphicsView->centerOn(center);
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
