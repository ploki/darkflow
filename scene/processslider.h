#ifndef PROCESSSLIDER_H
#define PROCESSSLIDER_H

#include <QGraphicsPathItem>
#include "processscene.h"

class Process;
class OperatorParameterSlider;
class Slider;

class ProcessSlider : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeSlider };

    explicit ProcessSlider(QRectF rect,
                           OperatorParameterSlider *slider,
                           Process *process,
                           ProcessNode *node);
    ~ProcessSlider();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const;

    void clicked(QPoint pos);

public slots:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void apply();
    void updateValue();
    void updateLoad();

private:
    Process *m_process;
    ProcessNode *m_node;
    OperatorParameterSlider *m_slider;
    QGraphicsTextItem *m_caption;
    QGraphicsTextItem *m_currentValue;
    bool m_mouseHover;
    Slider *m_sliderDialog;
};

#endif // PROCESSSLIDER_H
