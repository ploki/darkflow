#ifndef VISPOINT_H
#define VISPOINT_H

#include <QGraphicsPathItem>

class Visualization;

class VisPoint : public QGraphicsPathItem
{
public:
    enum { Type = QGraphicsItem::UserType + 1 };
    explicit VisPoint(QPointF pos, Visualization *vis, QGraphicsItem * parent = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    int type() const;
private:
    Visualization *m_vis;
};

#endif // VISPOINT_H
