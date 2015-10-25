#ifndef PROCESSNODE_H
#define PROCESSNODE_H

#include <QGraphicsPathItem>
#include <QPointF>

#include "processscene.h"

class Process;
class Operator;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class OperatorInput;

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

signals:

private slots:
    void operatorStateChanged();
    void closeButtonClicked();
    void passThroughClicked();
    void viewImageClicked();
    void viewParametersClicked();
    void playClicked();
    void abortClicked();

private:
    Operator *m_operator;
    Process *m_process;
    double m_completion;
    bool m_enabled;
    QGraphicsTextItem *m_caption;

    void addButtons(qreal size);
    void addPorts(QVector<OperatorInput*> inputs, qreal size);
};

#endif // PROCESSNODE_H
