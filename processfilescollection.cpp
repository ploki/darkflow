#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPainter>

#include "processfilescollection.h"
#include "process.h"
#include "processnode.h"
#include "operatorparameterfilescollection.h"
#include "filesselection.h"

#define PEN_WIDTH 2

ProcessFilesCollection::ProcessFilesCollection(QRectF rect,
                                 OperatorParameterFilesCollection *filesCollection,
                                 Process *process,
                                 ProcessNode *node) :
    QObject(NULL),
    QGraphicsPathItem(node),
    m_process(process),
    m_node(node),
    m_filesCollection(filesCollection),
    m_caption(NULL),
    m_currentValue(NULL),
    m_mouseHover(false),
    m_selectionDialog(new FilesSelection(NULL))
{
    setPen(QPen(Qt::black, PEN_WIDTH));
    setBrush(QBrush(Qt::gray));
    QPainterPath pp;
    QRectF pathRect(rect.x()+rect.width()/2, rect.y(), rect.width()/2, rect.height());
    QRectF captionRect(rect.x(), rect.y(), rect.width()/2, rect.height());
    pp.addRect(pathRect);
    setPath(pp);

    m_caption = new QGraphicsTextItem(filesCollection->caption(),this);
    m_caption->setPos(captionRect.center()-m_caption->boundingRect().center());

    m_currentValue= new QGraphicsTextItem(filesCollection->currentValue(),this);
    m_currentValue->setPos(pathRect.center()-m_currentValue->boundingRect().center());
    //connect(dropdown, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged(QString)));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    connect(m_selectionDialog, SIGNAL(accepted()), this, SLOT(selectionAccepted()));
    connect(m_selectionDialog, SIGNAL(rejected()), this, SLOT(selectionRejected()));
}

ProcessFilesCollection::~ProcessFilesCollection()
{
    delete m_selectionDialog;
}

void ProcessFilesCollection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

int ProcessFilesCollection::type() const
{
    return Type;
}

void ProcessFilesCollection::clicked(QPoint pos)
{
    m_selectionDialog->setSelection(m_filesCollection->collection());
    m_selectionDialog->show();
}

void ProcessFilesCollection::selectionAccepted()
{
    m_filesCollection->setCollection(m_selectionDialog->getSelection());
    m_selectionDialog->clearSelection();
    updateValue();
}

void ProcessFilesCollection::selectionRejected()
{
    m_selectionDialog->clearSelection();
}


void ProcessFilesCollection::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_mouseHover=true;
    update();
}

void ProcessFilesCollection::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_mouseHover=false;
    update();
}

void ProcessFilesCollection::updateValue()
{
    m_currentValue->setPlainText(m_filesCollection->currentValue());
    m_currentValue->setPos(boundingRect().center()-m_currentValue->boundingRect().center());
    m_currentValue->update();
}
