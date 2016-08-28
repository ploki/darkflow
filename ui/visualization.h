/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
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
class GraphicsViewInteraction;

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
    void zoomChanged(qreal factor);
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
    GraphicsViewInteraction *graphicsViewInteraction;

    void clearAllTabs();
    void updateTabs();
    void updateTabsWithPhoto();
    void updateTabsWithOutput();
    void updateColorLabels(const QPointF& pos);
    void updateTagsTable();
    void setInputControlEnabled(bool v);
    bool eventFilter(QObject *obj, QEvent *event);
    void drawROI();
    void storeROI();
    void reloadPoints();
    bool clearPoints(Tool tool);
    void addPoint(QPointF scenePos, int pointNumber);
    void removePoints(QPointF scenePos);

};

#endif // VISUALIZATION_H
