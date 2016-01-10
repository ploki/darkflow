#include <QPainter>

#include "processnode.h"
#include "processslider.h"
#include "slider.h"
#include "operatorparameterslider.h"
#include "operator.h"

#define PEN_WIDTH 2

ProcessSlider::ProcessSlider(QRectF rect,
                             OperatorParameterSlider *slider,
                             Process *process,
                             ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_slider(slider),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false),
    m_sliderDialog(new Slider(m_slider->windowCaption(),m_slider->unit(),
                              m_slider->scale(),m_slider->numberSet(),
                              m_slider->getMin(), m_slider->getMax(),
                              m_slider->value(),m_slider->hardMin(),
                              m_slider->hardMax(),m_slider->parametersFilter(),
                              NULL))
{
    setPen(QPen(Qt::black, PEN_WIDTH));
    setBrush(QBrush(Qt::gray));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2, rect.y(), rect.width()/2, rect.height());
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(m_slider->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());

    m_currentValue= new QGraphicsTextItem(m_slider->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    connect(m_sliderDialog, SIGNAL(updated()), this, SLOT(updateValue()));
    connect(m_slider, SIGNAL(updated()), this, SLOT(updateLoad()));
    updateValue();

}

ProcessSlider::~ProcessSlider()
{
    delete m_sliderDialog;
}

void ProcessSlider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

int ProcessSlider::type() const
{
    return Type;
}

void ProcessSlider::clicked(QPoint pos)
{
    if ( !m_sliderDialog->isVisible() ) {
        QPoint newPos = pos;
        newPos.setX(newPos.x() - m_sliderDialog->size().width()/2);
        newPos.setY(newPos.y() - m_sliderDialog->size().height()/2);
        m_sliderDialog->move(newPos.x(), newPos.y());
        m_sliderDialog->show();
    }
    m_sliderDialog->activateWindow();
    m_sliderDialog->raise();
}

void ProcessSlider::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=true;
    update();
}

void ProcessSlider::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}

void ProcessSlider::apply()
{
    m_node->m_operator->play();
}

void ProcessSlider::updateValue()
{
    m_slider->setUnit(m_sliderDialog->getUnit());
    m_slider->setScale(m_sliderDialog->getScale());
    m_slider->setMin(m_sliderDialog->getMin());
    m_slider->setMax(m_sliderDialog->getMax());
    m_slider->setValue(m_sliderDialog->getValue());

    m_currentValue->setPlainText(m_sliderDialog->currentValue());
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}

void ProcessSlider::updateLoad()
{
    m_sliderDialog->loadValues(m_slider->unit(),
                               m_slider->scale(),
                               m_slider->getMin(),
                               m_slider->getMax(),
                               m_slider->value());
}
