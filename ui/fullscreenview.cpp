#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QKeyEvent>

#include <cmath>

#include "fullscreenview.h"
#include "ui_fullscreenview.h"

FullScreenView::FullScreenView(QGraphicsScene *scene, QWidget *parent) :
    QMainWindow(parent),
    m_scene(scene),
    ui(new Ui::FullScreenView),
    m_zoomLevel(0)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->graphicsView->installEventFilter(this);
    ui->graphicsView->viewport()->installEventFilter(this);
}

bool FullScreenView::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::Wheel ) {
        int delta = dynamic_cast<QWheelEvent*>(event)->delta();
        const int min = -20;
        const int max = 20;
        if(delta > 0)
            m_zoomLevel+=1;
        else
            m_zoomLevel-=1;
        m_zoomLevel=qMax(qMin(m_zoomLevel,max),min);
        zoomUpdate();
        event->accept();
        return true;
    }
    else if ( event->type() == QEvent::KeyRelease ) {
        int key = dynamic_cast<QKeyEvent*>(event)->key();
        if ( key == Qt::Key_Escape ) {
            hide();
            event->accept();
            return true;
        }
        else if ( key == Qt::Key_1 ) {
            m_zoomLevel = 0;
            zoomUpdate();
            event->accept();
            return true;
        }

    }
    return QMainWindow::eventFilter(obj, event);
}

void FullScreenView::zoomUpdate()
{
    qreal factor = pow(2.,qreal(m_zoomLevel)/5.);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setTransform(QTransform(factor, 0., 0., factor, 0, 0));

}


