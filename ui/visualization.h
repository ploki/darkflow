#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QMainWindow>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsPathItem;
class TableTagsRow;

namespace Ui {
class Visualization;
}

class Operator;
class OperatorOutput;
class Photo;

class Visualization : public QMainWindow
{
    Q_OBJECT
public:
    explicit Visualization(Operator *op, QWidget *parent = 0);
    ~Visualization();

signals:
    void operatorNameChanged(QString text);

public slots:
    void nameChanged(QString text);
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
    void playClicked();

    void histogramParamsChanged();
    void curveParamsChanged();
    void clearTags();
    void tags_buttonAddClicked();
    void tags_buttonRemoveClicked();
    void tags_buttonResetClicked();
    void toolChanged(int idx);

private slots:
    void rubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);
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
    QVector<TableTagsRow*> m_tags;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
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


    void clearAllTabs();
    void updateTabs();
    void updateTabsWithPhoto();
    void updateTabsWithOutput();
    void updateVisualizationZoom();
    void updateVisualizationFitVisible();
    void updateTagsTable();
    void setInputControlEnabled(bool v);
    bool eventFilter(QObject *obj, QEvent *event);
    void drawROI();
    void storeROI();
};

#endif // VISUALIZATION_H
