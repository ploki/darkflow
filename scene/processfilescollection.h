#ifndef PROCESSFILESCOLLECTION_H
#define PROCESSFILESCOLLECTION_H

#include <QGraphicsPathItem>
#include <QRect>
#include "processscene.h"

class OperatorParameterFilesCollection;
class Process;
class ProcessNode;
class QGraphicsTextItem;
class FilesSelection;

class ProcessFilesCollection : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeFilesCollection };

    explicit ProcessFilesCollection(QRectF rect,
                    OperatorParameterFilesCollection *dropdown,
                    Process *process,
                    ProcessNode *node);
    ~ProcessFilesCollection();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const;

    void clicked(QPoint pos);

public slots:
    void selectionAccepted();
    void selectionRejected();
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void updateValue();

private:
    Process *m_process;
    ProcessNode *m_node;
    OperatorParameterFilesCollection *m_filesCollection;
    QGraphicsTextItem *m_caption;
    QGraphicsTextItem *m_currentValue;
    bool m_mouseHover;
    FilesSelection *m_selectionDialog;
};

#endif // PROCESSFILESCOLLECTION_H
