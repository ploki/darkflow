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

ProcessPort::ProcessPort(qreal x, qreal y,
                         const QString &portName,
                         ProcessPort::PortType portType,
                         Process *process, ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_portName(portName),
    m_portType(portType)
{
    qreal w,h;
    setPen(QPen(Qt::black));
    QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(m_portName);
    w = textItem->boundingRect().width();
    h = textItem->boundingRect().height();

    setPen(QPen(Qt::darkYellow));
    setBrush(QBrush(Qt::yellow));
    QVector<QPoint> points;

    switch (portType) {
    case ProcessPort::InputPort:
    case ProcessPort::InputOnePort:
        textItem->setPos(x+h,y-h*.25);
        points.push_back(QPoint(x+0,y+0));
        points.push_back(QPoint(x+h/2,y+0));
        points.push_back(QPoint(x+h,y+h*.25));
        points.push_back(QPoint(x+h/2,y+h*.5));
        points.push_back(QPoint(x+0,y+h*.5));
        points.push_back(QPoint(x+0,y+0));
        break;
    case ProcessPort::OutputPort:
        textItem->setPos(x-h-w,y-h*.25);
        points.push_back(QPoint(x-0,y+0));
        points.push_back(QPoint(x-h/2,y+0));
        points.push_back(QPoint(x-h,y+h*.25));
        points.push_back(QPoint(x-h/2,y+h*.5));
        points.push_back(QPoint(x-0,y+h*.5));
        points.push_back(QPoint(x-0,y+0));
    }


    QPolygon poly(points);
    QPainterPath pp;
    pp.addPolygon(poly);
    setPath(pp);
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
