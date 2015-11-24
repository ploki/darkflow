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
#include "tabletagsrow.h"
#include "tablewidgetitem.h"

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
    m_tags()
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

    QStringList headers;
    headers.push_back("Key");
    headers.push_back("Value");
    ui->table_tags->setRowCount(0);
    ui->table_tags->setColumnCount(2);
    ui->table_tags->setHorizontalHeaderLabels(headers);
    ui->table_tags->horizontalHeader()->setStretchLastSection(true);

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
        if ( m_operator->isTagOverrided(m_photo->getIdentity(), it.key()) )
            row->setValue(m_operator->getTagOverrided(m_photo->getIdentity(), it.key() ), true);
    }
    if ( m_operator->photoTagsExists(m_photo->getIdentity()) ) {
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
                identity = identity.split(":").first();
                int count = ++seen[identity];
                if ( count > 1 ) {
                    identity+=QString(":%0").arg(count-1);
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
    ui->widget_visualization->setPixmap(QPixmap());
    ui->widget_curve->setPixmap(QPixmap());
    ui->widget_histogram->setPixmap(QPixmap());
    m_output = NULL;
    m_photo = NULL;
    m_photoIsInput = false;

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
    if ( m_photoIsInput ) setInputControlEnabled(true);
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
