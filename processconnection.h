#ifndef PROCESSCONNECTION_H
#define PROCESSCONNECTION_H

#include <QGraphicsPathItem>
#include "processscene.h"

class Process;
class ProcessPort;
class ProcessNode;

class ProcessConnection : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeConnection };

    explicit ProcessConnection(ProcessPort *port,
                      Process *process);
    ~ProcessConnection();

    void updateDanglingPath(QPointF danglingPos);
    void updateConnectedPath();

    void setInputPort(ProcessPort *port);
    int type() const;

private slots:
    void portChanged();
    void portDestroyed(QObject*);

private:
    ProcessPort *m_outPort;
    ProcessPort *m_inPort;
    void updatePath(QPointF outPos, QPointF inPos);
};

#endif // PROCESSCONNECTION_H
