#ifndef PROCESSCONNECTION_H
#define PROCESSCONNECTION_H

#include <QGraphicsPathItem>
#include <QJsonObject>

#include "processscene.h"

class Process;
class ProcessPort;
class ProcessNode;

class ProcessConnection : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeConnection };

    explicit ProcessConnection(ProcessPort *port);
    ~ProcessConnection();

    void updateDanglingPath(QPointF danglingPos);
    void updateConnectedPath();

    bool setInputPort(ProcessPort *port);
    void unsetInputPort();
    void detach();
    int type() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QJsonObject save();

private slots:
    void portChanged();
    void portDestroyed(QObject*);

private:
    ProcessPort *m_outPort;
    ProcessPort *m_inPort;
    void updatePath(QPointF outPos, QPointF inPos);
};

#endif // PROCESSCONNECTION_H
