#ifndef PROCESSBUTTON_H
#define PROCESSBUTTON_H

#include <QGraphicsPathItem>
#include "processscene.h"

class Process;
class ProcessNode;

class ProcessButton : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeButton };

    explicit ProcessButton(qreal x, qreal y,
                           const QString& text,
                           Process *process,
                           ProcessNode *node,
                           Qt::GlobalColor vividColor=Qt::green,
                           Qt::GlobalColor color=Qt::darkGreen);
    ~ProcessButton();
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    int type() const;

signals:
    void buttonClicked();

public slots:
    void resetMouse();
    void mousePress();
    void mouseRelease();
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
private:
    Process *m_process;
    ProcessNode *m_node;
    QString m_text;
    bool m_mouseHover;
    bool m_mousePress;
    Qt::GlobalColor m_vividColor;
    Qt::GlobalColor m_color;
};

#endif // PROCESSBUTTON_H
