#include "vispoint.h"
#include <QPainterPath>
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <visualization.h>

VisPoint::VisPoint(QPointF pos,
                   Visualization *vis,
                   QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    m_vis(vis)
{
    qreal x = pos.x();
    qreal y = pos.y();
    QPainterPath pp;
    pp.addRoundedRect(x-3,y-3,6.,6.,1.,1.);
    pp.moveTo(x-10,y);
    pp.lineTo(x+10,y);
    pp.moveTo(x,y-10);
    pp.lineTo(x,y+10);
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
