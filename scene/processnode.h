#ifndef PROCESSNODE_H
#define PROCESSNODE_H

#include <QGraphicsPathItem>
#include <QPointF>
#include <QSet>
#include <QJsonObject>
#include <QVector>

#include "processscene.h"

class Process;
class ProcessConnection;
class ProcessPort;
class Operator;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class OperatorInput;
class OperatorOutput;
class OperatorParameter;
class Visualization;

class ProcessNode : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeNode };

    explicit ProcessNode(QPointF pos,
                         Operator *op,
                         Process *process,
                         QGraphicsItem *parent = 0);
    ~ProcessNode();
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    int type() const;
    void addConnection(ProcessConnection *connection);
    void removeConnection(ProcessConnection *connection);
    QJsonObject save();

    ProcessPort *inPort(int idx);
    ProcessPort *outPort(int idx);

signals:

private slots:
    void operatorStateChanged();
    void closeButtonClicked();
    void passThroughClicked();
    void visualizationClicked();
    void playClicked();
    void abortClicked();
    void operatorNameChanged(QString text);

public:
    Operator *m_operator;
private:
    Process *m_process;
    double m_completion;
    bool m_enabled;
    QGraphicsTextItem *m_caption;
    QSet<ProcessConnection*> m_connections;
    QVector<ProcessPort*> m_inPorts;
    QVector<ProcessPort*> m_outPorts;
    Visualization *m_visualization;

    void addButtons(qreal size);
    void addPorts(QVector<OperatorOutput*>& outputs,
                  QVector<OperatorInput*>& inputs,
                  qreal size);
    void addParameters(QVector<OperatorParameter*>& parameters, qreal size, qreal offset);
};

#endif // PROCESSNODE_H
