#ifndef FULLSCREENVIEW_H
#define FULLSCREENVIEW_H

#include <QMainWindow>


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
    void zoomUpdate();

signals:

public slots:
private:
    QGraphicsScene *m_scene;
    Ui::FullScreenView *ui;
    int m_zoomLevel;
};

#endif // FULLSCREENVIEW_H
