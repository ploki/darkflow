#ifndef PROCESSNODE_H
#define PROCESSNODE_H

#include <QGraphicsPathItem>
#include <QPointF>
#include <QSet>
#include <QJsonObject>

#include "processscene.h"

class Process;
class ProcessConnection;
class Operator;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class OperatorInput;
class OperatorOutput;
class OperatorParameter;

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

signals:

private slots:
    void operatorStateChanged();
    void closeButtonClicked();
    void passThroughClicked();
    void viewImageClicked();
    void viewParametersClicked();
    void playClicked();
    void abortClicked();

public:
    Operator *m_operator;
private:
    Process *m_process;
    double m_completion;
    bool m_enabled;
    QGraphicsTextItem *m_caption;
    QSet<ProcessConnection*> m_connections;

    void addButtons(qreal size);
    void addPorts(QVector<OperatorOutput*>& outputs,
                  QVector<OperatorInput*>& inputs,
                  qreal size);
    void addParameters(QVector<OperatorParameter*>& parameters, qreal size, qreal offset);
};

#endif // PROCESSNODE_H
