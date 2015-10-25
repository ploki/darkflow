#include <QPainter>

#include "processprogressbar.h"
#include "process.h"
#include "processnode.h"

#define PEN_WIDTH 2

ProcessProgressBar::ProcessProgressBar(QRectF rect, Process *process, ProcessNode *node) :
    QObject(NULL),
    QGraphicsRectItem(QRectF(rect.x()+PEN_WIDTH,rect.y()+PEN_WIDTH,
                             rect.width()-PEN_WIDTH*2,rect.height()-PEN_WIDTH*2),node),
    m_process(process),
    m_node(node),
    m_overlay(NULL),
    m_progress(0),
    m_complete(1)
{
    m_overlay = new QGraphicsRectItem(boundingRect(),this);
    m_overlay->setBrush(Qt::green);
    m_overlay->setPen(QPen(Qt::black,2));
    setBrush(Qt::darkRed);
    setPen(QPen(Qt::black, PEN_WIDTH));
    setFlags(QGraphicsItem::ItemIsSelectable);
}

ProcessProgressBar::~ProcessProgressBar()
{

}

void ProcessProgressBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRect(boundingRect());
    m_overlay->setRect(QRect(boundingRect().x(),boundingRect().y(),
                             boundingRect().width()*double(m_progress)/double(m_complete) ,boundingRect().height()));
}

int ProcessProgressBar::type() const
{
    return Type;
}

void ProcessProgressBar::progress(int p, int c)
{
    m_progress = p;
    m_complete = c;
    update();
}
