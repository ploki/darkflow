#ifndef FULLSCREENVIEW_H
#define FULLSCREENVIEW_H

#include <QMainWindow>

class GraphicsViewInteraction;

namespace Ui {
class FullScreenView;
}

class QGraphicsView;
class QGraphicsScene;

class FullScreenView : public QMainWindow
{
    Q_OBJECT
public:
    explicit FullScreenView(QGraphicsScene *scene, QWidget *parent = 0);
    bool eventFilter(QObject *, QEvent *);
    void zoomUpdate(qreal factor);

signals:

public slots:
private:
    QGraphicsScene *m_scene;
    Ui::FullScreenView *ui;
    GraphicsViewInteraction *graphicsViewInteraction;
};

#endif // FULLSCREENVIEW_H
