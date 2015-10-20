#include <QPainterPath>
#include <QPainter>
#include <QString>
#include <QGraphicsTextItem>

#include "process.h"
#include "processnode.h"
#include "processbutton.h"
#include "processport.h"
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
    qreal w = 200, h = 100;
    qreal xradius=3, yradius=3;
    setPen(QPen(Qt::black));

    pp.addRoundedRect(x, y, w, h, xradius, yradius);
    setFlag(QGraphicsItem::ItemIsMovable);

    m_caption = new QGraphicsTextItem(this);
    m_caption->setPlainText(op->getClassIdentifier());
    m_caption->setPos(x, y);
    pp.addRect(x,y+m_caption->boundingRect().height(),w,0);
    setPath(pp);

    connect(m_process, SIGNAL(stateChanged()), this, SLOT(operatorStateChanged()));

    const qreal margin=2;
    qreal offset=margin;
    ProcessButton *buttClose = new ProcessButton(x+w-offset,y,"❌",m_process, this, Qt::red, Qt::darkRed);
    connect(buttClose, SIGNAL(buttonClicked()), this, SLOT(closeButtonClicked()));
    offset+=margin+buttClose->boundingRect().width();

    ProcessButton *buttPassThrough= new ProcessButton(x+w-offset,y,"●",m_process, this);
    connect(buttPassThrough, SIGNAL(buttonClicked()), this, SLOT(passThroughClicked()));
    offset+=margin+buttPassThrough->boundingRect().width();

    ProcessButton *buttViewImage= new ProcessButton(x+w-offset,y,"⚉",m_process, this);
    connect(buttViewImage, SIGNAL(buttonClicked()), this, SLOT(viewImageClicked()));
    offset+=margin+buttViewImage->boundingRect().width();

    ProcessButton *buttViewParameters= new ProcessButton(x+w-offset,y,"☷",m_process, this);
    connect(buttViewParameters, SIGNAL(buttonClicked()), this, SLOT(viewParametersClicked()));


    offset=margin;
    ProcessButton *buttPlay= new ProcessButton(x,y+h,"▶",m_process, this);
    connect(buttPlay, SIGNAL(buttonClicked()), this, SLOT(playClicked()));
    offset+=margin+buttPlay->boundingRect().width();
    buttPlay->setPos(10+offset,
                     -20-buttPlay->boundingRect().height());

    ProcessButton *buttAbort= new ProcessButton(x,y+h,"■",m_process, this, Qt::magenta, Qt::darkMagenta);
    connect(buttAbort, SIGNAL(buttonClicked()), this, SLOT(playClicked()));
    offset+=margin+buttAbort->boundingRect().width();
    buttAbort->setPos(10+offset,
                     -20-buttAbort->boundingRect().height());

    ProcessPort *port1 = new ProcessPort(x,y+30,"In", ProcessPort::InputPort, m_process, this);
    offset=margin+port1->boundingRect().height();
    ProcessPort *port2 = new ProcessPort(x,y+30+offset,"sub", ProcessPort::InputOnePort, m_process, this);

    ProcessPort *out = new ProcessPort(x+w,y+30,"Out", ProcessPort::OutputPort, m_process, this);
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
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(QBrush(Qt::darkGray));
    painter->drawPath(path());
}

int ProcessNode::type() const { return Type; }

void ProcessNode::operatorStateChanged()
{

}

void ProcessNode::closeButtonClicked()
{
    deleteLater();
}

void ProcessNode::passThroughClicked()
{

}

void ProcessNode::viewImageClicked()
{

}

void ProcessNode::viewParametersClicked()
{

}

void ProcessNode::playClicked()
{

}

void ProcessNode::abortClicked()
{

}

