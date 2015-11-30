#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QPixmap>
#include <QEvent>
#include <QScrollBar>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>

#include <QWindow>

#include <Magick++.h>

#include "visualization.h"
#include "ui_visualization.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "treephotoitem.h"
#include "treeoutputitem.h"
#include "tabletagsrow.h"
#include "tablewidgetitem.h"
#include "vispoint.h"
#include "fullscreenview.h"

Visualization::Visualization(Operator *op, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Visualization),
    m_operator(op),
    m_output(NULL),
    m_photo(NULL),
    m_zoomLevel(ZoomFitVisible),
    m_zoom(0),
    m_currentPhoto(),
    m_currentOutput(0),
    m_photoIsInput(false),
    m_tags(),
    m_scene(new QGraphicsScene),
    m_pixmapItem(new QGraphicsPixmapItem),
    m_lastMouseScreenPosition(),
    m_points(),
    m_roi(0),
    m_roi_p1(),
    m_roi_p2(),
    m_tool(ToolNone),
    m_fullScreenView(new FullScreenView(m_scene, this))
{
    ui->setupUi(this);
    ui->operatorName->setText(m_operator->getName());
    ui->operatorClass->setText(m_operator->getClassIdentifier());
    setWindowTitle(m_operator->getName());
    connect(ui->operatorName, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->tree_photos, SIGNAL(itemSelectionChanged()), this, SLOT(photoSelectionChanged()));
    connect(this, SIGNAL(operatorNameChanged(QString)), m_operator, SLOT(setName(QString)));
    connect(m_operator, SIGNAL(upToDate()), this, SLOT(upToDate()));
    connect(m_operator, SIGNAL(outOfDate()), this, SLOT(outOfDate()), Qt::QueuedConnection);

    updateTreeviewPhotos();
    updateVisualizationZoom();
    setInputControlEnabled(false);
    clearTags();
    toolChanged(0);

    QStringList headers;
    headers.push_back("Key");
    headers.push_back("Value");
    ui->table_tags->setRowCount(0);
    ui->table_tags->setColumnCount(2);
    ui->table_tags->setHorizontalHeaderLabels(headers);
    ui->table_tags->horizontalHeader()->setStretchLastSection(true);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->adjustSize();
    m_scene->addItem(m_pixmapItem);
    m_scene->installEventFilter(this);
    ui->graphicsView->installEventFilter(this);
    connect(ui->graphicsView, SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)), this, SLOT(rubberBandChanged(QRect,QPointF,QPointF)));
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

Visualization::~Visualization()
{
    clearTags();
    delete ui;
}


void Visualization::zoomFitVisible()
{
    //qDebug("action fit!");
    m_zoomLevel=ZoomFitVisible;
    updateVisualizationZoom();
}

void Visualization::zoomHalf()
{
    m_zoomLevel=ZoomHalf;
    updateVisualizationZoom();
}

void Visualization::zoomOne()
{
    m_zoomLevel=ZoomOne;
    updateVisualizationZoom();
}

void Visualization::zoomDouble()
{
    m_zoomLevel=ZoomDouble;
    updateVisualizationZoom();
}

void Visualization::zoomCustom()
{
    m_zoomLevel=ZoomCustom;
    updateVisualizationZoom();
}

void Visualization::zoomPlus()
{
    if ( m_zoom < 20)
        ++m_zoom;
    ui->radio_zoomCustom->click();
    updateVisualizationZoom();
}

void Visualization::zoomMinus()
{
    if ( m_zoom > -20 )
        --m_zoom;
    ui->radio_zoomCustom->click();
    updateVisualizationZoom();
}

void Visualization::expChanged()
{
    if ( m_photo && m_photo->isComplete() ) {
        ui->value_exp->setText(QString("%0 EV").arg(qreal(ui->slider_exp->value())/100.));
        int exposure = ui->slider_exp->value();
        qreal gamma, x0;
        switch(ui->combo_gamma->currentIndex()) {
        default:
            qWarning("Unknown combo_gamma selection");
        case 0: //Linear
            gamma = 1.; x0 = 0; break;
        case 1: //sRGB
            gamma = 2.4L; x0 = 0.00304L; break;
        case 2: //IUT BT.709
            gamma = 2.222L; x0 = 0.018L; break;
        case 3: //POW-2;
            gamma = 2.L; x0 = 0.; break;
        }
        m_pixmapItem->setPixmap(m_photo->imageToPixmap(gamma, x0, pow(2.,qreal(exposure)/100.)));
        m_scene->setSceneRect(0,0,m_photo->image().columns(),m_photo->image().rows());
    }
}

void Visualization::upToDate()
{
    updateTreeviewPhotos();
    this->raise();
}

void Visualization::outOfDate()
{
    QTreeWidget *tree = ui->tree_photos;
    QTreeWidgetItemIterator it(tree);
    while (*it) {
        if ( TreePhotoItem * photoItem = dynamic_cast<TreePhotoItem*>(*it) ) {
            if (!photoItem->photo().isComplete() )
                photoItem->photo().setUndefined();
        }
        ++it;
    }
    if ( this->isVisible() && ui->checkBox_autoPlay->isChecked() ) {
        qDebug(QString(m_operator->uuid() + " Vis requests play").toLatin1());
        m_operator->play();
    }
}

void Visualization::playClicked()
{
    m_operator->play();
}

void Visualization::fullScreenViewClicked()
{
    m_fullScreenView->showFullScreen();
    //m_fullScreenView->showMaximized();
}

void Visualization::histogramParamsChanged()
{
    if ( m_photo && m_photo->isComplete() ) {
        Photo::HistogramScale scale;
        Photo::HistogramGeometry geometry;
        switch ( ui->combo_log->currentIndex()) {
        default:
            qWarning("Unknown combo_log histogram selection");
        case 0:
            scale = Photo::HistogramLinear; break;
        case 1:
            scale = Photo::HistogramLogarithmic; break;
        }
        switch ( ui->combo_surface->currentIndex()) {
        default:
            qWarning("Unknown combo_surface selection");
        case 0:
            geometry = Photo::HistogramLines; break;
        case 1:
            geometry = Photo::HistogramSurfaces; break;
        }

        ui->widget_histogram->setPixmap(m_photo->histogramToPixmap(scale, geometry));
    }
}

void Visualization::curveParamsChanged()
{
    if ( m_photo && m_photo->isComplete() ) {
        Photo::CurveView cv;
        switch(ui->combo_scale->currentIndex()) {
        default:
        case 0: //Linear
            cv = Photo::sRGB_EV; break;
        case 1: //Level
            cv = Photo::sRGB_Level; break;
        case 2: //log2
            cv = Photo::Log2; break;
        }

        ui->widget_curve->setPixmap(m_photo->curveToPixmap(cv));
    }
}

void Visualization::clearTags()
{
    foreach(TableTagsRow *tagRow, m_tags)
        delete tagRow;
    m_tags.clear();
    //ui->table_tags->clear();
}

void Visualization::tags_buttonAddClicked()
{
    if (!m_photo) return;
    m_tags.push_back(new TableTagsRow(m_photo->getIdentity(), "New key", "value", TableTagsRow::FromOperator, ui->table_tags, m_operator));
}

void Visualization::tags_buttonRemoveClicked()
{
    if (!m_photo) return;
    QList<QTableWidgetItem *> items = ui->table_tags->selectedItems();
    foreach(QTableWidgetItem *item, items ) {
        TableTagsRow *row = dynamic_cast<TableWidgetItem*>(item)->tableRow();
        bool removed = row->remove();
        if ( !removed ) {
            int idx = m_tags.indexOf(row);
            if (-1 != idx ) {
                delete row;
                m_tags.remove(idx);
            }
            else {
                qWarning("row not found in m_tags");
            }
        }
    }
}

void Visualization::tags_buttonResetClicked()
{
    if (!m_photo) return;
    QList<QTableWidgetItem *> items = ui->table_tags->selectedItems();
    foreach(QTableWidgetItem *item, items ) {
        TableTagsRow *row = dynamic_cast<TableWidgetItem*>(item)->tableRow();
        row->reset();
    }
}

void Visualization::toolChanged(int idx)
{
    switch(idx) {
    default:
        qWarning("Unknown tool combo index");
    case 0: m_tool = ToolNone; break;
    case 1: m_tool = ToolROI; break;
    case 2: m_tool = Tool1Point; break;
    case 3: m_tool = Tool2Points; break;
    case 4: m_tool = Tool3Points; break;
    case 5: m_tool = ToolNPoints; break;
    }
    switch (m_tool) {
    case ToolNone:
        ui->graphicsView->setCursor(Qt::ArrowCursor);
        ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
        break;
    case ToolROI:
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
        break;
    default:
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
        break;
    }
}

void Visualization::updateVisualizationZoom()
{
    switch(m_zoomLevel) {
    case ZoomFitVisible:
        updateVisualizationFitVisible();
        return;
        break;
    case ZoomHalf:
        m_zoom=-5;
        break;
    case ZoomOne:
        m_zoom=0;
        break;
    case ZoomDouble:
        m_zoom=5;
        break;
    case ZoomCustom: default: break;
    }
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    qreal factor = pow(2,qreal(m_zoom)/5);
    transformView(factor);
}

void Visualization::updateVisualizationFitVisible()
{
    if ( m_zoomLevel == ZoomFitVisible && m_photo ) {
        ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QSize widgetSize = ui->graphicsView->size();
        int w = m_photo->image().columns();
        int h = m_photo->image().rows();
        double factor;
        if ( double(widgetSize.width())/double(widgetSize.height()) <=
             double(w)/double(h) ) {
            factor = double(widgetSize.width()-2)/double(w);
        }
        else {
            factor = double(widgetSize.height()-2)/double(h);
        }
        transformView(factor);
    }

}

void Visualization::transformView(qreal factor)
{
    if (factor >= 1 )
        m_pixmapItem->setTransformationMode(Qt::FastTransformation);
    else
        m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setTransform(QTransform(factor, 0., 0., factor, 0, 0));

}

void Visualization::updateTagsTable()
{
    if ( !m_photo )
        return;
    clearTags();
    QMap<QString, QString> tags = m_photo->tags();
    for(QMap<QString, QString>::iterator it = tags.begin() ;
        it != tags.end() ;
        ++it ) {
        TableTagsRow *row = new TableTagsRow(m_photo->getIdentity(), it.key(), it.value(), TableTagsRow::FromPhoto, ui->table_tags, m_operator);
        m_tags.push_back(row);
        if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), it.key()) )
            row->setValue(m_operator->getTagOverrided(m_photo->getIdentity(), it.key() ), true);
    }
    if ( m_photoIsInput && m_operator->photoTagsExists(m_photo->getIdentity()) ) {
        QMap<QString, QString> tags = m_operator->photoTags(m_photo->getIdentity());
        for(QMap<QString, QString>::iterator it = tags.begin() ;
            it != tags.end() ;
            ++it ) {
            bool found = false;
            foreach(TableTagsRow *row, m_tags) {
                if ( row->getKey() == it.key() ) {
                    found = true;
                    break;
                }
            }
            if ( !found ) {
                m_tags.push_back(new TableTagsRow(m_photo->getIdentity(), it.key(), it.value(), TableTagsRow::FromOperator, ui->table_tags, m_operator));
            }
        }
    }
}

void Visualization::nameChanged(QString text)
{
    setWindowTitle(text);
    emit operatorNameChanged(text);
}

void Visualization::updateTreeviewPhotos()
{
    QTreeWidget *tree = ui->tree_photos;
    QList<QTreeWidgetItem*> selectedItems = tree->selectedItems();
    if ( selectedItems.count() == 1 ) {
        if ( TreePhotoItem * photoItem = dynamic_cast<TreePhotoItem*>(selectedItems.first()) ) {
            m_currentPhoto = photoItem->photo().getIdentity();
            m_currentOutput = dynamic_cast<TreeOutputItem*>(photoItem->parent())->output();
        }
    }
    tree->clear();

    QMap<QString, int> seen;

    QTreeWidgetItem *tree_inputs = new QTreeWidgetItem(tree);
    tree_inputs->setText(0, "Inputs:");
    tree_inputs->setFont(0,QFont("Sans", 14));
    tree_inputs->setBackground(0,QBrush(Qt::lightGray));
    foreach(OperatorInput *input, m_operator->m_inputs) {
        QTreeWidgetItem *tree_input = new QTreeWidgetItem(tree_inputs);
        tree_input->setText(0, input->name());
        tree_input->setFont(0, QFont("Sans", 12));
        tree_input->setBackground(0, QBrush(Qt::green));
        foreach(OperatorOutput *source, input->sources()) {
            QTreeWidgetItem *tree_source = new TreeOutputItem(source, TreeOutputItem::Source, tree_input);
            foreach(Photo photo, source->m_result) {
                if ( !photo.isComplete() )
                    qWarning("source photo is not complete");
                QString identity = photo.getIdentity();
                identity = identity.split("|").first();
                int count = ++seen[identity];
                if ( count > 1 ) {
                    identity+=QString("|%0").arg(count-1);
                    photo.setIdentity(identity);
                }
                TreePhotoItem *item = new TreePhotoItem(photo, TreePhotoItem::Input, tree_source);
                if ( identity == m_currentPhoto &&
                     source == m_currentOutput ) {
                    item->setSelected(true);
                }
            }

        }
    }

    QTreeWidgetItem *tree_outputs = new QTreeWidgetItem(tree);
    tree_outputs->setText(0, "outputs:");
    tree_outputs->setFont(0, QFont("Sans", 14));
    tree_outputs->setBackground(0, QBrush(Qt::lightGray));
    foreach(OperatorOutput *output, m_operator->m_outputs) {
        QTreeWidgetItem *tree_output = new TreeOutputItem(output,TreeOutputItem::Sink,tree_outputs);
        foreach(const Photo& photo, output->m_result) {
            if ( !photo.isComplete() )
                qWarning("output photo is not complete");
            TreePhotoItem *item = new TreePhotoItem(photo, TreePhotoItem::Output, tree_output);
            if ( photo.getIdentity() == m_currentPhoto &&
                 output == m_currentOutput ) {
                item->setSelected(true);
            }
        }
    }

    tree->expandAll();
}

void Visualization::photoSelectionChanged()
{
    QList<QTreeWidgetItem*> items = ui->tree_photos->selectedItems();

    if ( items.count() > 1 ) {
        qDebug(QString("Invalid photo selection in visualization tree view: sel count %0").arg(items.count()).toLatin1());
        return;
    }

    clearAllTabs();
    foreach(QTreeWidgetItem *item, items ) {
        switch(item->type()) {
        case TreePhotoItem::Type: {
            TreePhotoItem *photoItem = dynamic_cast<TreePhotoItem*>(item);
            m_photo = &photoItem->photo();
            m_photoIsInput = photoItem->isInput();
            updateTabs();
            return;
        }
        case TreeOutputItem::Type:
            m_output = dynamic_cast<TreeOutputItem*>(item)->output();
            updateTabs();
            return;
        default:
            return;
        }
    }
}

void Visualization::clearAllTabs()
{
    m_pixmapItem->setPixmap(QPixmap());
    ui->widget_curve->setPixmap(QPixmap());
    ui->widget_histogram->setPixmap(QPixmap());
    drawROI();
    m_output = NULL;
    m_photo = NULL;
    m_photoIsInput = false;

    clearPoints(ToolNone);
    clearTags();
    setInputControlEnabled(false);
}

void Visualization::updateTabs()
{
    if (m_photo) updateTabsWithPhoto();
    if (m_output) updateTabsWithOutput();
}

void Visualization::updateTabsWithPhoto()
{
    expChanged();
    curveParamsChanged();
    histogramParamsChanged();
    updateVisualizationZoom();
    updateTagsTable();
    ui->value_width->setText(QString::number(m_photo->image().columns()));
    ui->value_height->setText(QString::number(m_photo->image().rows()));
    if ( m_photoIsInput ) setInputControlEnabled(true);
    QString roiTag = m_photo->getTag("ROI");
    if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), "ROI") ) {
        roiTag = m_operator->getTagOverrided(m_photo->getIdentity(), "ROI");
    }
    if ( roiTag.count() ) {
        QStringList coord = roiTag.split(',');
        if ( coord.count() == 4 ) {
            m_roi_p1.setX(coord[0].toDouble());
            m_roi_p1.setY(coord[1].toDouble());
            m_roi_p2.setX(coord[2].toDouble());
            m_roi_p2.setY(coord[3].toDouble());
        }
    }
    drawROI();
    m_roi_p1.setX(0);
    m_roi_p1.setY(0);
    m_roi_p2 = m_roi_p1;

    clearPoints(ToolNone);
    QString pointsTag = m_photo->getTag("POINTS");
    if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), "POINTS") ) {
        pointsTag = m_operator->getTagOverrided(m_photo->getIdentity(), "POINTS");
    }
    QStringList coords = pointsTag.split(';');
    foreach(QString coord, coords) {
        QStringList coordStr = coord.split(',');
        if (coordStr.count() != 2) continue;
        addPoint(QPointF(coordStr[0].toDouble(),coordStr[1].toDouble()));
    }
}

void Visualization::updateTabsWithOutput()
{

}


void Visualization::setInputControlEnabled(bool v)
{
    ui->table_tags->setEnabled(v);
    ui->combo_tool->setEnabled(v);
    ui->tag_buttonAdd->setEnabled(v);
    ui->tag_buttonRemove->setEnabled(v);
    ui->tag_buttonReset->setEnabled(v);
}
void Visualization::rubberBandChanged(QRect, QPointF p1, QPointF p2)
{
    if ( !m_photo )
        return;
    if ( m_roi ) {
        m_scene->removeItem(m_roi);
        m_roi = NULL;
    }
    if ( p1.isNull() && p2.isNull() ) {
        drawROI();
        storeROI();
        ui->statusBar->showMessage("");
    }
    else
        ui->statusBar->showMessage(QString("Selection: x1=%0, y1=%1, x2=%2, y2=%3").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()));
    m_roi_p1 = p1;
    m_roi_p2 = p2;
}

bool Visualization::eventFilter(QObject *obj, QEvent *event)
{
    switch(event->type()) {
    case QEvent::GraphicsSceneMousePress: {
        QGraphicsSceneMouseEvent *me =
                dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        m_lastMouseScreenPosition = me->screenPos();
    }
        break;
    case QEvent::GraphicsSceneMouseRelease: {
        QGraphicsSceneMouseEvent *me =
                dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        if ( m_lastMouseScreenPosition == me->screenPos() &&
             m_tool != ToolNone && m_tool != ToolROI &&
             m_photoIsInput ) {
            removePoints(me->scenePos());
          if ( me->button() == Qt::LeftButton ) {
                addPoint(me->scenePos());
                clearPoints(m_tool);
                storePoints();
          }
        }
    }
        break;
    case QEvent::Resize: {
        updateVisualizationFitVisible();
    }
        break;
    case QEvent::KeyRelease: {
        fullScreenViewClicked();
        event->accept();
        return true;
    }
        break;
    default:break;

    }
    return QMainWindow::eventFilter(obj, event);
}

void Visualization::drawROI()
{
    if ( m_roi ) {
        m_scene->removeItem(m_roi);
        m_roi = NULL;
    }
    if ( m_roi_p1.isNull() && m_roi_p2.isNull() )
        return;
    QPointF p1=m_roi_p1;
    QPointF p2=m_roi_p2;
    QPointF p3(p1);
    QPointF p4(p2);
    p3.setX(p2.x());
    p4.setX(p1.x());
    QPainterPath pp;
    pp.moveTo(p1);
    pp.lineTo(p4);
    pp.lineTo(p2);
    pp.lineTo(p3);
    pp.lineTo(p1);
    m_roi = m_scene->addPath(pp, QPen(Qt::green, 1));//, QBrush(QColor(10,10,10,10)));
}

void Visualization::storeROI()
{
    QString roiTag = QString("%0,%1,%2,%3")
            .arg(m_roi_p1.x())
            .arg(m_roi_p1.y())
            .arg(m_roi_p2.x())
            .arg(m_roi_p2.y());
    m_operator->setTagOverride(m_photo->getIdentity(),"ROI", roiTag);
}

void Visualization::clearPoints(Tool tool)
{
    int maxPoints = 0;
    switch(tool) {
    case ToolNone:
    case ToolROI:
        maxPoints = 0;
        break;
    case Tool1Point:
        maxPoints = 1;
        break;
    case Tool2Points:
        maxPoints = 2;
        break;
    case Tool3Points:
        maxPoints = 3;
        break;
    case ToolNPoints:
        maxPoints = 100000;
        break;
    default: break;
    }

    while(m_points.count() > maxPoints ) {
        m_scene->removeItem(m_points.last());
        m_points.pop_back();
    }
}

void Visualization::addPoint(QPointF scenePos)
{
    VisPoint *point = new VisPoint(scenePos, this);
    m_scene->addItem(point);
    m_points.prepend(point);
}

void Visualization::removePoints(QPointF scenePos)
{
    bool modified = false;
    QList<QGraphicsItem*> items = m_scene->items(QRectF(scenePos - QPointF(1,1), QSize(3,3)));
    foreach(QGraphicsItem *item, items) {
        if ( item->type() == VisPoint::Type ) {
            VisPoint *point = dynamic_cast<VisPoint*>(item);
            m_scene->removeItem(point);
            m_points.removeOne(point);
            modified = true;
        }
    }
    if ( modified )
        storePoints();
}

void Visualization::storePoints()
{
    if ( !m_photo ) return;
    QString pointsTag;
    foreach(VisPoint *point, m_points) {
        if ( pointsTag.count() )
            pointsTag += ";";
        point->scenePos();
        pointsTag += QString::number(point->mapToScene(point->boundingRect().center()).x()) +
                "," +
                QString::number(point->mapToScene(point->boundingRect().center()).y());
    }
    if ( pointsTag.count() )
        m_operator->setTagOverride(m_photo->getIdentity(),"POINTS", pointsTag);
    else
        m_operator->resetTagOverride(m_photo->getIdentity(),"POINTS");
}
