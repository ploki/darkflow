#ifndef PROCESSBUTTON_H
#define PROCESSBUTTON_H

#include <QGraphicsRectItem>
#include "processscene.h"

class Process;
class ProcessNode;

class ProcessButton : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeButton };

    typedef enum { Undef,
                   Play,
                   Abort,
                   Display,
                   Close,
                   Ghost } ButtonType;

    explicit ProcessButton(QRectF rect,
                           ButtonType type,
                           Process *process,
                           ProcessNode *node);
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
    ButtonType m_type;
    bool m_mouseHover;
    bool m_mousePress;

};

#endif // PROCESSBUTTON_H
