#include <QPainterPath>
#include <QPainter>
#include <QString>
#include <QGraphicsTextItem>

#include "process.h"
#include "processnode.h"
#include "operator.h"

ProcessNode::ProcessNode(qreal x, qreal y,
                         Operator *op,
                         Process *process,
                         QGraphicsItem *parent) :
    QObject(NULL),
    QGraphicsPathItem(parent),
    m_operator(op),
    m_process(process),
    m_caption(NULL)
{
    QPainterPath pp;
    int w = 200, h = 100;
    int xradius=3, yradius=3;
    setPen(QPen(Qt::black));

    pp.addRoundedRect(x, y, w, h, xradius, yradius);
    setFlag(QGraphicsItem::ItemIsMovable);

    m_caption = new QGraphicsTextItem(this);
    m_caption->setPlainText(op->getClassIdentifier());
    m_caption->setPos(x, y);
    pp.addRect(x,y+m_caption->boundingRect().height(),w,0);
    setPath(pp);

    connect(m_process, SIGNAL(stateChanged()), this, SLOT(operatorStateChanged()));
}

ProcessNode::~ProcessNode()
{
    //disconnect(m_process, SIGNAL(stateChanged()), this, SLOT(operatorStateChanged()));
    delete m_operator;
}

void ProcessNode::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setBrush(QBrush(Qt::darkGray));
    painter->drawPath(path());
}

int ProcessNode::type() const { return Type; }

void ProcessNode::operatorStateChanged()
{

}

