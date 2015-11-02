#include <QPainter>
#include <QPainterPath>
#include <QGraphicsItem>
#include <QCursor>

#include "processconnection.h"
#include "processnode.h"
#include "processport.h"

#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"

ProcessConnection::ProcessConnection(ProcessPort *port,
                                     Process *process) :
    QObject(NULL),
    QGraphicsPathItem(NULL),
    m_outPort(port),
    m_inPort(NULL)
{
    //setPos(port->anchorScenePos());
    setPen(QPen(Qt::black,4));
    setBrush(Qt::NoBrush);
    setZValue(-1);
    connect(m_outPort, SIGNAL(positionChanged()), this, SLOT(portChanged()));
    connect(m_outPort, SIGNAL(destroyed(QObject*)), this, SLOT(portDestroyed(QObject*)));
    //m_outPort->m_node->addConnection(this);
    /*
     * if move from anywhere:
     * if this flag is not set, scene move instead of connection when detached from inPort
     */
    //setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setCursor(QCursor(Qt::ArrowCursor));
}

ProcessConnection::~ProcessConnection()
{
    //m_outPort->m_node->removeConnection(this);
    if (m_inPort)
        Operator::operator_disconnect(m_outPort->m_node->m_operator->m_outputs[m_outPort->portIdx()],
                m_inPort->m_node->m_operator->m_inputs[m_inPort->portIdx()]);
}

int ProcessConnection::type() const
{
    return ProcessConnection::Type;
}

void ProcessConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setBrush(brush());
    if ( m_inPort )
        painter->setPen(pen());
    else
        painter->setPen(QPen(Qt::darkRed,4));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path());
}

QJsonObject ProcessConnection::save()
{
    QJsonObject obj;
    obj["outPortUuid"] = m_outPort->m_node->m_operator->getUuid();
    obj["outPortIdx"] = m_outPort->portIdx();
    obj["inPortUuid"] = m_inPort->m_node->m_operator->getUuid();
    obj["inPortIdx"] = m_inPort->portIdx();
    return obj;
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
    dx = qMin(500., qMax(50.,(outPos-inPos).manhattanLength()/2));
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

    Operator::operator_connect(m_outPort->m_node->m_operator->m_outputs[m_outPort->portIdx()],
            m_inPort->m_node->m_operator->m_inputs[m_inPort->portIdx()]);
    m_inPort->m_node->addConnection(this);
    m_outPort->m_node->addConnection(this);
    updateConnectedPath();
}

void ProcessConnection::unsetInputPort()
{
    disconnect(m_inPort, SIGNAL(positionChanged()), this, SLOT(portChanged()));
    disconnect(m_inPort, SIGNAL(destroyed(QObject*)), this, SLOT(portDestroyed(QObject*)));
    Operator::operator_disconnect(m_outPort->m_node->m_operator->m_outputs[m_outPort->portIdx()],
            m_inPort->m_node->m_operator->m_inputs[m_inPort->portIdx()]);
    m_inPort->m_node->removeConnection(this);
    m_outPort->m_node->removeConnection(this);
    m_inPort = NULL;
}

void ProcessConnection::detach()
{
    unsetInputPort();
    deleteLater();
}

