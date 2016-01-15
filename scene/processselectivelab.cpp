#include <QPainter>

#include "processnode.h"
#include "processselectivelab.h"
#include "selectivelab.h"
#include "operatorparameterselectivelab.h"
#include "operator.h"

#define PEN_WIDTH 2

ProcessSelectiveLab::ProcessSelectiveLab(QRectF rect,
                                         OperatorParameterSelectiveLab *selectiveLab,
                                         Process *process,
                                         ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_selectiveLab(selectiveLab),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false),
    m_selectiveLabDialog(new SelectiveLab(m_selectiveLab->windowCaption(),
                                          m_selectiveLab->hue(),
                                          m_selectiveLab->coverage(),
                                          m_selectiveLab->strict(),
                                          m_selectiveLab->level(),
                                          m_selectiveLab->displayGuide(),
                                          m_selectiveLab->previewEffect(),
                                          m_node->m_operator, NULL))
{
    setPen(QPen(Qt::black, PEN_WIDTH));
    setBrush(QBrush(Qt::gray));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2, rect.y(), rect.width()/2, rect.height());
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(m_selectiveLab->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());

    m_currentValue= new QGraphicsTextItem(m_selectiveLab->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    connect(m_selectiveLabDialog, SIGNAL(updated()), this, SLOT(updateValue()));
    connect(m_selectiveLab, SIGNAL(updated()), this, SLOT(updateLoad()));
    updateValue();
}

ProcessSelectiveLab::~ProcessSelectiveLab()
{
    delete m_selectiveLabDialog;
}

void ProcessSelectiveLab::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

int ProcessSelectiveLab::type() const
{
    return Type;
}

void ProcessSelectiveLab::clicked(QPoint pos)
{
    if ( !m_selectiveLabDialog->isVisible() ) {
        QPoint newPos = pos;
        newPos.setX(newPos.x() - m_selectiveLabDialog->size().width()/2);
        newPos.setY(newPos.y() - m_selectiveLabDialog->size().height()/2);
        m_selectiveLabDialog->move(newPos.x(), newPos.y());
        m_selectiveLabDialog->show();
    }
    m_selectiveLabDialog->activateWindow();
    m_selectiveLabDialog->raise();
}

void ProcessSelectiveLab::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=true;
    update();
}

void ProcessSelectiveLab::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_mouseHover=false;
    update();
}

void ProcessSelectiveLab::apply()
{
    m_node->m_operator->play();
}

void ProcessSelectiveLab::updateValue()
{
    m_selectiveLab->setHue(m_selectiveLabDialog->hue());
    m_selectiveLab->setCoverage(m_selectiveLabDialog->coverage());
    m_selectiveLab->setStrict(m_selectiveLabDialog->strict());
    m_selectiveLab->setLevel(m_selectiveLabDialog->level());
    m_selectiveLab->setDisplayGuide(m_selectiveLabDialog->displayGuide());
    m_selectiveLab->setPreviewEffect(m_selectiveLabDialog->previewEffect());

    m_currentValue->setPlainText(m_selectiveLab->currentValue());
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}

void ProcessSelectiveLab::updateLoad()
{
    m_selectiveLabDialog->loadValues(m_selectiveLab->hue(),
                                     m_selectiveLab->coverage(),
                                     m_selectiveLab->strict(),
                                     m_selectiveLab->level(),
                                     m_selectiveLab->displayGuide(),
                                     m_selectiveLab->previewEffect());
}
