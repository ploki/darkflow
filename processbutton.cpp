#include <QPainterPath>
#include <QPainter>
#include <QString>
#include <QGraphicsTextItem>
#include <QEvent>
#include <QCursor>
#include "processbutton.h"
#include "process.h"
#include "processnode.h"

#define PEN_WIDTH 2

static QString buttonText(ProcessButton::ButtonType type)
{
    switch(type) {
    case ProcessButton::Play: return "▶";
    case ProcessButton::Abort:return "■";
    case ProcessButton::Display: return "☷";
    case ProcessButton::Close: return "❌";
    case ProcessButton::Ghost: return "⚉";
    }
}

static QPen buttonPen(ProcessButton::ButtonType type)
{
    switch(type) {
    case ProcessButton::Play: return QPen(Qt::darkGreen, PEN_WIDTH);
    case ProcessButton::Abort: return QPen(Qt::darkMagenta, PEN_WIDTH);
    case ProcessButton::Display: return QPen(Qt::darkYellow, PEN_WIDTH);
    case ProcessButton::Close: return QPen(Qt::darkRed, PEN_WIDTH);
    case ProcessButton::Ghost: return QPen(Qt::darkBlue, PEN_WIDTH);
    }
}

static QBrush buttonBrush(ProcessButton::ButtonType type)
{
    switch(type) {
    case ProcessButton::Play: return QBrush(Qt::green);
    case ProcessButton::Abort: return QBrush(Qt::magenta);
    case ProcessButton::Display: return QBrush(Qt::yellow);
    case ProcessButton::Close: return QBrush(Qt::red);
    case ProcessButton::Ghost: return QBrush(Qt::blue);
    }
}

ProcessButton::ProcessButton(QRectF rect,
                             ProcessButton::ButtonType type,
                             Process *process,
                             ProcessNode *node) :
    QObject(NULL),
    QGraphicsRectItem(QRectF(rect.x()+PEN_WIDTH,rect.y()+PEN_WIDTH,
                             rect.width()-PEN_WIDTH*2,rect.height()-PEN_WIDTH*2),node),
    m_process(process),
    m_node(node),
    m_type(type),
    m_mouseHover(false),
    m_mousePress(false)
{
    //qreal radius=3;

    setPen(QPen(Qt::black,PEN_WIDTH));
    setBrush(QBrush(Qt::darkGray));

    QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(buttonText(m_type));
    QPointF textCenter = textItem->boundingRect().center();
    QPointF boxCenter = boundingRect().center();
    textItem->setPos(boxCenter-textCenter);
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
        painter->setPen(buttonPen(m_type));
    else
        painter->setPen(pen());
    if (m_mousePress)
        painter->setBrush(buttonBrush(m_type));
    else
        painter->setBrush(brush());
//        painter->setBrush(Qt::transparent);
    painter->setRenderHint(QPainter::Antialiasing);
    //painter->drawPath(path());
    painter->drawRect(boundingRect());
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
