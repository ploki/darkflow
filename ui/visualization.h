#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QMainWindow>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsPathItem;
class VisPoint;
class TableTagsRow;
class FullScreenView;

namespace Ui {
class Visualization;
}

class Operator;
class OperatorOutput;
class Photo;
class QTreeWidgetItem;
class TreePhotoItem;

class Visualization : public QMainWindow
{
    Q_OBJECT
public:
    explicit Visualization(Operator *op, QWidget *parent);
    ~Visualization();

    void getViewGamma(qreal &gamma, qreal &x0) const;
    qreal getViewExposure() const;

signals:
    void operatorNameChanged(QString text);

public slots:
    void nameChanged(QString text);
    void nameReset();
    void updateTreeviewPhotos();
    void photoSelectionChanged();
    void zoomFitVisible();
    void zoomHalf();
    void zoomOne();
    void zoomDouble();
    void zoomCustom();
    void zoomPlus();
    void zoomMinus();
    void expChanged();
    void upToDate();
    void outOfDate();
    void stateChanged();
    void playClicked();
    void getInputsClicked();
    void fullScreenViewClicked();
    void saveViewClicked();

    void histogramParamsChanged();
    void curveParamsChanged();
    void clearTags();
    void tags_buttonAddClicked();
    void tags_buttonRemoveClicked();
    void tags_buttonResetClicked();
    void toolChanged(int idx);
    void treatmentChanged(int idx);
    void inputTypeChanged(int idx);
    void storePoints();
    void treeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column);

private slots:
    void rubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);
    void progress(int, int);
private:
    Ui::Visualization *ui;
    Operator *m_operator;
    OperatorOutput *m_output;
    Photo *m_photo;
    enum ZoomLevel {
        ZoomFitVisible,
        ZoomHalf,
        ZoomOne,
        ZoomDouble,
        ZoomCustom
    } m_zoomLevel;
    int m_zoom;
    QString m_currentPhoto;
    const OperatorOutput *m_currentOutput;
    bool m_photoIsInput;
    TreePhotoItem *m_photoItem;
    QVector<TableTagsRow*> m_tags;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    QPoint m_lastMouseScreenPosition;
    QList<VisPoint*> m_points;
    QGraphicsPathItem *m_roi;
    QPointF m_roi_p1;
    QPointF m_roi_p2;
    enum Tool {
        ToolNone,
        ToolROI,
        Tool1Point,
        Tool2Points,
        Tool3Points,
        ToolNPoints,
    } m_tool;
    FullScreenView *m_fullScreenView;

    void clearAllTabs();
    void updateTabs();
    void updateTabsWithPhoto();
    void updateTabsWithOutput();
    void updateVisualizationZoom();
    void updateVisualizationFitVisible();
    void transformView(qreal factor);
    void updateColorLabels(const QPointF& pos);
    void updateTagsTable();
    void setInputControlEnabled(bool v);
    bool eventFilter(QObject *obj, QEvent *event);
    void drawROI();
    void storeROI();
    void clearPoints(Tool tool);
    void addPoint(QPointF scenePos);
    void removePoints(QPointF scenePos);

};

#endif // VISUALIZATION_H
