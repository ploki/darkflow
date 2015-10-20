#ifndef PROCESSPORT_H
#define PROCESSPORT_H

#include <QGraphicsPathItem>

#include "processscene.h"

class Process;
class ProcessNode;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class ProcessPort : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    typedef enum { InputPort,
                   InputOnePort,
                   OutputPort } PortType;
    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypePort };

    explicit ProcessPort(qreal x, qreal y,
                         const QString& portName,
                         PortType portType,
                         Process *process,
                         ProcessNode *node);
    ~ProcessPort();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
    Process *m_process;
    QString m_portName;
    PortType m_portType;
};

#endif // PROCESSPORT_H
