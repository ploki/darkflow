#ifndef GRAPHICSVIEWINTERACTION_H
#define GRAPHICSVIEWINTERACTION_H

#include <QObject>

class QGraphicsView;

class GraphicsViewInteraction : public QObject
{
    Q_OBJECT

public:
    explicit GraphicsViewInteraction(QGraphicsView *graphicsView, QObject *parent);
    bool eventFilter(QObject *obj, QEvent *event);

    void fitVisible();
    void zoomSet(qreal factor);
    void zoomIn();
    void zoomOut();
signals:
    void zoomChanged(qreal factor);

private:
    void zoomApply(qreal factor);
    void zoomUpdate(qreal factor);
    QGraphicsView *m_graphicsView;
    qreal totalScaleFactor;
    qreal lastGestureFactor;
    bool zoomKeyPressed;
};

#endif // GRAPHICSVIEWINTERACTION_H
