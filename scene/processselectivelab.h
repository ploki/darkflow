#ifndef PROCESSSELECTIVELAB_H
#define PROCESSSELECTIVELAB_H

#include <QGraphicsPathItem>
#include "processscene.h"

class Process;
class OperatorParameterSelectiveLab;
class SelectiveLab;

class ProcessSelectiveLab : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeSelectiveLab };

    explicit ProcessSelectiveLab(QRectF rect,
                                 OperatorParameterSelectiveLab *selectiveLab,
                                 Process *process,
                                 ProcessNode *node);
    ~ProcessSelectiveLab();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const;

    void clicked(QPoint pos);

public slots:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void apply();
    void updateValue();
    void updateLoad();

signals:
    void updated();

private:
    Process *m_process;
    ProcessNode *m_node;
    OperatorParameterSelectiveLab *m_selectiveLab;
    QGraphicsTextItem *m_caption;
    QGraphicsTextItem *m_currentValue;
    bool m_mouseHover;
    SelectiveLab *m_selectiveLabDialog;

};

#endif // PROCESSSELECTIVELAB_H
