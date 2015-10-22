#include <QPainterPath>
#include <QGraphicsItem>

#include "processconnection.h"
#include "processnode.h"
#include "processport.h"

ProcessConnection::ProcessConnection(ProcessPort *port,
                                     Process *process) :
    QObject(NULL),
    QGraphicsPathItem(NULL),
    m_outPort(port)
{
    //setPos(port->anchorScenePos());
    setPen(QPen(Qt::black, 2));
    setBrush(Qt::NoBrush);
    setZValue(-1);
    connect(m_outPort, SIGNAL(positionChanged()), this, SLOT(portChanged()));
    connect(m_outPort, SIGNAL(destroyed(QObject*)), this, SLOT(portDestroyed(QObject*)));
}

ProcessConnection::~ProcessConnection()
{

}

int ProcessConnection::type() const
{
    return ProcessConnection::Type;
}

void ProcessConnection::portChanged()
{
    updateConnectedPath();
}

void ProcessConnection::portDestroyed(QObject *)
{
    deleteLater();
}

void ProcessConnection::updatePath(QPointF outPos, QPointF inPos)
{
    qreal dx;
    dx = qMin(500., qMax(100.,(outPos-inPos).manhattanLength()/2));
    QPainterPath pp;
    pp.moveTo(outPos);
    QPointF ctrl1(outPos.x() + dx, outPos.y());
    QPointF ctrl2(inPos.x() - dx, inPos.y());
    pp.cubicTo(ctrl1, ctrl2, inPos);
    setPath(pp);
}

void ProcessConnection::updateDanglingPath(QPointF dangling)
{
    QPointF outPos = m_outPort->anchorScenePos();
    updatePath(outPos, dangling);
}

void ProcessConnection::updateConnectedPath()
{
    QPointF outPos = m_outPort->anchorScenePos();
    QPointF inPos = m_inPort->anchorScenePos();
    updatePath(outPos, inPos);
}

void ProcessConnection::setInputPort(ProcessPort *port)
{
    m_inPort = port;
    connect(m_inPort, SIGNAL(positionChanged()), this, SLOT(portChanged()));
    connect(m_inPort, SIGNAL(destroyed(QObject*)), this, SLOT(portDestroyed(QObject*)));
    updateConnectedPath();
}

