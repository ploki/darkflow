#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPainter>

#include "processdropdown.h"
#include "process.h"
#include "processnode.h"
#include "operatorparameterdropdown.h"

#define PEN_WIDTH 2

ProcessDropDown::ProcessDropDown(QRectF rect,
                                 OperatorParameterDropDown *dropdown,
                                 Process *process,
                                 ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_dropdown(dropdown),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false)
{
    setPen(QPen(Qt::black, PEN_WIDTH));
    setBrush(QBrush(Qt::gray));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2, rect.y(), rect.width()/2, rect.height());
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(dropdown->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());

    m_currentValue= new QGraphicsTextItem(dropdown->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());
    connect(dropdown, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged(QString)));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

}

ProcessDropDown::~ProcessDropDown()
{

}

void ProcessDropDown::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    if (m_mouseHover)
        painter->setBrush(QBrush(Qt::white));
    else
        painter->setBrush(brush());
    painter->setPen(pen());
    painter->drawPath(path());

}

int ProcessDropDown::type() const
{
    return Type;
}

void ProcessDropDown::clicked(QPoint pos)
{
    m_dropdown->dropDown(pos);
}

void ProcessDropDown::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_mouseHover=true;
    update();
}

void ProcessDropDown::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_mouseHover=false;
    update();
}

void ProcessDropDown::valueChanged(const QString &value)
{
    m_currentValue->setPlainText(value);
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}
