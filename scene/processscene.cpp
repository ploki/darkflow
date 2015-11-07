#include "processscene.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include "processnode.h"

ProcessScene::ProcessScene(QWidget *parent) :
    QGraphicsScene(parent)
{

}

void ProcessScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    emit contextMenuSignal(event);
}

