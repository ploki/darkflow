#ifndef PROCESSDROPDOWN_H
#define PROCESSDROPDOWN_H

#include <QGraphicsPathItem>
#include <QRect>
#include "processscene.h"

class OperatorParameterDropDown;
class Process;
class ProcessNode;
class QGraphicsTextItem;

class ProcessDropDown : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeDropDown };

    explicit ProcessDropDown(QRectF rect,
                    OperatorParameterDropDown *dropdown,
                    Process *process,
                    ProcessNode *node);
    ~ProcessDropDown();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const;

    void clicked(QPoint pos);
public slots:

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private slots:
    void valueChanged(const QString& value);

private:
    Process *m_process;
    ProcessNode *m_node;
    OperatorParameterDropDown *m_dropdown;
    QGraphicsTextItem *m_caption;
    QGraphicsTextItem *m_currentValue;
    bool m_mouseHover;
};

#endif // PROCESSDROPDOWN_H
