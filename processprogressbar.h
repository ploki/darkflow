#ifndef PROCESSPROGRESSBAR_H
#define PROCESSPROGRESSBAR_H

#include <QGraphicsRectItem>
#include "processscene.h"

class Process;
class ProcessNode;

class ProcessProgressBar : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeProgressBar };

    explicit ProcessProgressBar(QRectF rect, Process *process, ProcessNode *node);
    ~ProcessProgressBar();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const;
public slots:
    void progress(int p, int c);

private:
    QGraphicsRectItem *m_overlay;
    Process *m_process;
    ProcessNode *m_node;
    int m_progress;
    int m_complete;

};

#endif // PROCESSPROGRESSBAR_H
