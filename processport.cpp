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
    setPen(QPen(Qt::black));
    QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(m_portName);
    qreal textW = textItem->boundingRect().width();
    qreal textH = textItem->boundingRect().height();

    qreal unit = textH/4;
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = rect.width();
    qreal h = rect.height();
    y+= (h-unit*2)/2;
    setPen(QPen(Qt::darkYellow));
    setBrush(QBrush(Qt::yellow));

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

