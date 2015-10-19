#ifndef PROCESSNODE_H
#define PROCESSNODE_H

#include <QGraphicsPathItem>

#include "processscene.h"

class Process;
class Operator;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class ProcessNode : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeNode };

    explicit ProcessNode(qreal x, qreal y,
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

};

#endif // PROCESSNODE_H
