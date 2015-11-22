#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QPixmap>

#include <Magick++.h>

#include "visualization.h"
#include "ui_visualization.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "treephotoitem.h"
#include "treeoutputitem.h"

Visualization::Visualization(Operator *op, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Visualization),
    m_operator(op),
    m_output(NULL),
    m_photo(NULL),
    m_zoomLevel(ZoomFitVisible),
    m_zoom(0),
    m_currentPhoto(),
    m_currentOutput(0)
{
    ui->setupUi(this);
    ui->operatorName->setText(m_operator->getName());
    ui->operatorClass->setText(m_operator->getClassIdentifier());
    setWindowTitle(m_operator->getName());
    connect(ui->operatorName, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->tree_photos, SIGNAL(itemSelectionChanged()), this, SLOT(photoSelectionChanged()));
    connect(this, SIGNAL(operatorNameChanged(QString)), m_operator, SLOT(setName(QString)));
    connect(m_operator, SIGNAL(upToDate()), this, SLOT(upToDate()));
    connect(m_operator, SIGNAL(outOfDate()), this, SLOT(outOfDate()));

    updateTreeviewPhotos();
    updateVisualizationZoom();
}

Visualization::~Visualization()
{
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
    if ( m_zoom < 10)
        ++m_zoom;
    ui->radio_zoomCustom->click();
    updateVisualizationZoom();
}

void Visualization::zoomMinus()
{
    if ( m_zoom > -10 )
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
        ui->widget_visualization->setPixmap(m_photo->imageToPixmap(gamma, x0, pow(2.,qreal(exposure)/100.)));
    }
}

void Visualization::upToDate()
{
    updateTreeviewPhotos();
    this->raise();
}

void Visualization::outOfDate()
{
    bool one_defined = false;
    Q_UNUSED(one_defined);
    QTreeWidget *tree = ui->tree_photos;
    QTreeWidgetItemIterator it(tree);
    while (*it) {
        if ( TreePhotoItem * photoItem = dynamic_cast<TreePhotoItem*>(*it) ) {
            if (photoItem->photo().isComplete() )
                one_defined = true;
            photoItem->photo().setUndefined();
        }
        ++it;
    }
    if ( /* one_defined &&*/ this->isVisible() ) {
        qDebug(QString(m_operator->m_uuid + " Vis requests play").toLatin1());
        m_operator->play();
    }
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

void Visualization::updateVisualizationZoom()
{
    //qDebug("updateVis");
    if ( ui->widget_visualization->pixmap() == NULL )
        return;
    //qDebug("pixmap defined");
    switch(m_zoomLevel) {
    case ZoomFitVisible:
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
    qreal zoom_factor = pow(2,qreal(m_zoom)/5);
    if ( m_zoomLevel == ZoomFitVisible ) {
        //qDebug("proceed fit vis");
        ui->scrollArea_visualization->setWidgetResizable(false);
        ui->widget_visualization->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
        QSize rect = ui->scrollArea_visualization->viewport()->size();
        QSize size = ui->widget_visualization->pixmap()->size();
        size.scale(rect, Qt::KeepAspectRatio);
        ui->widget_visualization->resize(size);
        //ui->widget_visualization->adjustSize();
    }
    else {
        //qDebug("proceed zoom");
        ui->scrollArea_visualization->setWidgetResizable(true);
        ui->widget_visualization->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        ui->widget_visualization->resize(ui->widget_visualization->pixmap()->size()*zoom_factor);
        //ui->widget_visualization->adjustSize();
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
            foreach(const Photo& photo, source->m_result) {
                if ( !photo.isComplete() )
                    qWarning("source photo is not complete");
                TreePhotoItem *item = new TreePhotoItem(photo, tree_source);
                if ( photo.getIdentity() == m_currentPhoto &&
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
            TreePhotoItem *item = new TreePhotoItem(photo, tree_output);
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
        case TreePhotoItem::Type:
            m_photo = &dynamic_cast<TreePhotoItem*>(item)->photo();
            updateTabs();
            return;
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
    ui->widget_visualization->setPixmap(QPixmap());
    ui->widget_curve->setPixmap(QPixmap());
    ui->widget_histogram->setPixmap(QPixmap());
    m_output = NULL;
    m_photo = NULL;
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
}

void Visualization::updateTabsWithOutput()
{

}

