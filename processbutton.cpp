#include <QPainterPath>
#include <QPainter>
#include <QString>
#include <QGraphicsTextItem>
#include <QEvent>
#include <QCursor>
#include "processbutton.h"
#include "process.h"
#include "processnode.h"

ProcessButton::ProcessButton(qreal x,
                             qreal y,
                             const QString &text,
                             Process *process,
                             ProcessNode *node,
                             Qt::GlobalColor vividColor,
                             Qt::GlobalColor color) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_text(text),
    m_mouseHover(false),
    m_mousePress(false),
    m_vividColor(vividColor),
    m_color(color)
{
    qreal w,h;
    qreal radius=3;
    setPen(QPen(Qt::darkBlue));
    QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(m_text);
    w = textItem->boundingRect().width();
    h = textItem->boundingRect().height();
    textItem->setPos(x-w,y);
    QPainterPath pp;
    pp.addRoundedRect(x-w+2, y+2, w-4, h-4, radius, radius);
    setPath(pp);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
}

ProcessButton::~ProcessButton()
{

}

void ProcessButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_mouseHover)
        painter->setPen(m_color);
    else
        painter->setPen(Qt::darkBlue);
    if (m_mousePress)
        painter->setBrush(m_vividColor);
    else
        painter->setBrush(Qt::transparent);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path());
}


int ProcessButton::type() const
{
    return ProcessButton::Type;
}

void ProcessButton::resetMouse()
{
    m_mousePress=false;
    update();
}

void ProcessButton::mousePress()
{
    m_mousePress=true;
}

void ProcessButton::mouseRelease()
{
    if ( m_mousePress )
    {
        emit buttonClicked();
    }
    m_mousePress=false;
}

void ProcessButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
   Q_UNUSED(event);
   m_mouseHover=true;
   update();
}

void ProcessButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}
