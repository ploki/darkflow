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
#include <QFileDialog>

#include <Magick++.h>
#include <cmath>

#include "graphicsviewinteraction.h"
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
#include "console.h"
#include "preferences.h"
#include "darkflow.h"
#include "cielab.h"
#include "process.h"

Visualization::Visualization(Operator *op, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Visualization),
    m_operator(op),
    m_output(NULL),
    m_photo(NULL),
    m_currentPhoto(),
    m_currentOutput(0),
    m_photoIsInput(false),
    m_photoItem(0),
    m_tags(),
    m_scene(new QGraphicsScene),
    m_pixmapItem(new QGraphicsPixmapItem),
    m_lastMouseScreenPosition(),
    m_points(),
    m_roi(0),
    m_roi_p1(),
    m_roi_p2(),
    m_tool(ToolNone),
    m_fullScreenView(new FullScreenView(m_scene, this)),
    graphicsViewInteraction(0)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
#ifdef Q_OS_OSX
    /* Don't let osx permit this window to be fullscreen.
     * I can't figure out why multiple fullscreen views
     * behave baddly
     */
    setWindowFlags(Qt::Tool);
#else
    /* Don't know why it is now required to permit fullscreen
     * at least on linux
     */
    setWindowFlags(Qt::Window);
#endif
    ui->operatorName->setText(m_operator->getName());
    ui->operatorNameReset->setText(m_operator->getLocalizedClassIdentifier());
    ui->operatorNameReset->setToolTip(m_operator->getClassIdentifier());
    setWindowTitle(m_operator->getName());
    m_fullScreenView->setWindowTitle(m_operator->getName());
    connect(ui->operatorName, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->tree_photos, SIGNAL(itemSelectionChanged()), this, SLOT(photoSelectionChanged()));
    connect(this, SIGNAL(operatorNameChanged(QString)), m_operator, SLOT(setName(QString)));
    connect(m_operator, SIGNAL(upToDate()), this, SLOT(upToDate()));
    connect(m_operator, SIGNAL(outOfDate()), this, SLOT(outOfDate()), Qt::QueuedConnection);
    connect(m_operator, SIGNAL(progress(int,int)), this, SLOT(progress(int,int)));
    updateColorLabels(QPointF(-1,-1));
    updateTreeviewPhotos();
    setInputControlEnabled(false);
    clearTags();
    toolChanged(0);

    QStringList headers;
    headers.push_back(tr("Key"));
    headers.push_back(tr("Value"));
    ui->table_tags->setRowCount(0);
    ui->table_tags->setColumnCount(2);
    ui->table_tags->setHorizontalHeaderLabels(headers);
    ui->table_tags->horizontalHeader()->setStretchLastSection(true);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->adjustSize();
    ui->combo_gamma->setCurrentIndex(preferences->getCurrentTarget());
    m_scene->addItem(m_pixmapItem);
    m_scene->installEventFilter(this);
    graphicsViewInteraction = new GraphicsViewInteraction(ui->graphicsView, this);
//    ui->graphicsView->installEventFilter(this);
    connect(ui->graphicsView, SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)), this, SLOT(rubberBandChanged(QRect,QPointF,QPointF)));
    connect(ui->tree_photos, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(treeWidgetItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(graphicsViewInteraction, SIGNAL(zoomChanged(qreal)), this, SLOT(zoomChanged(qreal)));
    ui->graphicsView->setMouseTracking(true);
    ui->value_EV_R->setAlignment(Qt::AlignRight);
    ui->value_EV_G->setAlignment(Qt::AlignRight);
    ui->value_EV_B->setAlignment(Qt::AlignRight);
    graphicsViewInteraction->fitVisible();
}

Visualization::~Visualization()
{
    clearTags();
    delete ui;
}


void Visualization::zoomFitVisible()
{
    graphicsViewInteraction->fitVisible();
}

void Visualization::zoomHalf()
{
    graphicsViewInteraction->zoomSet(.5);
}

void Visualization::zoomOne()
{
    graphicsViewInteraction->zoomSet(1);
}

void Visualization::zoomDouble()
{
    graphicsViewInteraction->zoomSet(2);
}

void Visualization::zoomCustom()
{
}

void Visualization::zoomPlus()
{
    ui->radio_zoomCustom->click();
    graphicsViewInteraction->zoomIn();
}

void Visualization::zoomMinus()
{
    ui->radio_zoomCustom->click();
    graphicsViewInteraction->zoomOut();
}

void Visualization::zoomChanged(qreal factor)
{
    if (factor<0)
        ui->radio_fitVisible->click();
    else if (factor==1)
        ui->radio_zoom1->click();
    else
        ui->radio_zoomCustom->click();
}

void Visualization::getViewGamma(qreal &gamma, qreal &x0) const
{
    gamma = 1.;
    x0 = 0;
    switch(ui->combo_gamma->currentIndex()) {
    default:
        dflWarning(tr("Visualization: Unknown combo_gamma selection"));
    case 0: //As Input
        gamma = 1.; x0 = 0; break;
    case 1: //sRGB
        gamma = 2.4L; x0 = 0.00304L; break;
    case 2: //IUT BT.709
        gamma = 2.222L; x0 = 0.018L; break;
    case 3: //POW-2;
        gamma = 2.L; x0 = 0.; break;
    }
}
qreal Visualization::getViewExposure() const
{
    return qreal(ui->slider_exp->value())/100.;
}

void Visualization::expChanged()
{
    if ( m_photo && m_photo->isComplete() ) {
        qreal exposure = getViewExposure();
        qreal gamma, x0;
        getViewGamma(gamma, x0);
        ui->value_exp->setText(tr("%0 EV").arg(exposure));
        m_pixmapItem->setPixmap(m_photo->imageToPixmap(gamma, x0, pow(2.,exposure)));
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
        dflDebug(tr("%0 Visualization requests play").arg(m_operator->uuid()));
        m_operator->play();
    }
}

void Visualization::stateChanged()
{
    updateTreeviewPhotos();
}

void Visualization::playClicked()
{
    ui->checkBox_autoPlay->setChecked(true);
    m_operator->play();
}

void Visualization::getInputsClicked()
{
    ui->checkBox_autoPlay->setChecked(false);
    m_operator->refreshInputs();
}

void Visualization::fullScreenViewClicked()
{
#ifdef Q_OS_OSX
    /* No fullscreen for the fullscreen view on os X,
     * maximized is a good trade-off
     */
    m_fullScreenView->showMaximized();
#else
    m_fullScreenView->showFullScreen();
#endif
}

void Visualization::saveViewClicked()
{
    QString filters[][3] = {
        tr("JPEG Images (*.jpg *.jpeg)"), "JPG", ".jpg:.jpeg",
        tr("TIFF Images (*.tif *.tiff)"), "TIFF", ".tif:.tiff",
        tr("FITS Images (*.fits)"), "FITS", ".fits"
    };
    QString filter;
    for (int i = 0, s = sizeof(filters)/sizeof(*filters) ; i < s ; ++i ) {
        if ( 0 != i )
            filter += ";;";
        filter += filters[i][0];
    }
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save view image"),
                                                    this->m_operator->m_process->baseDirectory(),
                                                    filter,
                                                    &selectedFilter, 0);
    if ( filename.isEmpty() )
        return;
    qreal exposure = getViewExposure();
    qreal gamma, x0;
    getViewGamma(gamma, x0);
    QString magick;
    bool filterFound = false;
    for (int i = 0, s = sizeof(filters)/sizeof(*filters) ; i < s ; ++i ) {
        if ( selectedFilter == filters[i][0] ) {
            filterFound = true;
            magick = filters[i][1];
            QStringList extensions = filters[i][2].split(":");
            bool extFound = false;
            foreach(const QString &extension, extensions) {
                if (filename.endsWith(extension, Qt::CaseInsensitive))
                    extFound = true;
            }
            if (!extFound) {
                filename+=filters[i][2].split(":").first();
            }
        }
    }
    if (filterFound)
        m_photo->saveImage(filename, magick, gamma, x0, pow(2.,exposure));
    else
        dflCritical(tr("Unspecified file type for '%0'").arg(filename));
}

void Visualization::histogramParamsChanged()
{
    if ( m_photo && m_photo->isComplete() ) {
        Photo::HistogramScale scale;
        Photo::HistogramGeometry geometry;
        switch ( ui->combo_log->currentIndex()) {
        default:
            dflWarning(tr("Visualization: Unknown combo_log histogram selection"));
        case 0:
            scale = Photo::HistogramLinear; break;
        case 1:
            scale = Photo::HistogramLogarithmic; break;
        }
        switch ( ui->combo_surface->currentIndex()) {
        default:
            dflWarning(tr("Visualization: Unknown combo_surface selection"));
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
    m_tags.push_back(new TableTagsRow(m_photo->getIdentity(), tr("New key"), tr("value"), TableTagsRow::FromOperator, ui->table_tags, m_operator));
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
                dflWarning(tr("Visualization: row not found in m_tags"));
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
    Qt::TransformationMode transformationMode = Qt::FastTransformation;

    switch(idx) {
    default:
        dflWarning(tr("Visualization: Unknown tool combo index"));
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
        transformationMode = Qt::SmoothTransformation;
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
    if (m_pixmapItem)
        m_pixmapItem->setTransformationMode(transformationMode);
}

void Visualization::treatmentChanged(int idx)
{
    TreePhotoItem::PhotoType type;
    if ( !m_photo || (m_photo && !m_photoIsInput) )
        return;
    QString identity = m_photo->getIdentity();
    bool from_photo = !m_operator->isTagOverrided(identity, TAG_TREAT);
    QString value;
    switch(idx) {
    case 0: value = TAG_TREAT_REGULAR; type = TreePhotoItem::InputEnabled; break;
    case 1: value = TAG_TREAT_REFERENCE; type = TreePhotoItem::InputReference; break;
    case 2: value = TAG_TREAT_DISCARDED; type = TreePhotoItem::InputDisabled; break;
    case 3: value = TAG_TREAT_ERROR; type = TreePhotoItem::InputError; break;
    default:
        dflWarning(tr("Visualization: Unknown type"));
        type = TreePhotoItem::InputDisabled;
    }

    QString photoTag = m_photo->getTag(TAG_TREAT);
    QString treatTag = photoTag;
    if ( !from_photo ) {
        treatTag = m_operator->getTagOverrided(m_photo->getIdentity(), TAG_TREAT);
    }

    if ( treatTag == value )
        return;
    if ( photoTag == value || ( value == TAG_TREAT_REGULAR && photoTag=="" ) )
        m_operator->resetTagOverride(identity, TAG_TREAT);
    else
        m_operator->setTagOverride(identity, TAG_TREAT, value);
    m_photoItem->setType(type);
}

void Visualization::inputTypeChanged(int idx)
{
    if ( !m_photo || (m_photo && !m_photoIsInput) )
        return;
    QString value;
    switch(idx) {
    default:
        dflWarning(tr("Visualization: Unknown pixel scale idx"));
    case 0: value = TAG_SCALE_LINEAR; break;
    case 1: value = TAG_SCALE_NONLINEAR; break;
    case 2: value = TAG_SCALE_HDR; break;
    }
    m_operator->setTagOverride(m_photo->getIdentity(), TAG_SCALE, value);
}

void Visualization::updateColorLabels(const QPointF& pos)
{
    using Magick::Quantum;
    QVector<qreal> rgb(3);
    bool clearStatus = true;

    if ( pos.x() >= 0 && pos.y() >= 0 && m_photo ) {
        clearStatus = false;
        rgb = m_photo->pixelColor(pos.x(), pos.y());
    }
    QString rStr(tr("R: %0"));
    QString gStr(tr("G: %0"));
    QString bStr(tr("B: %0"));
    if ( m_photo && m_photo->getScale() == Photo::HDR ) {
        rStr = tr("r %0").arg(rgb[0], 5, 'f', 5, QChar(' ')).left(8);
        gStr = tr("g %0").arg(rgb[1], 5, 'f', 5, QChar(' ')).left(8);
        bStr = tr("b %0").arg(rgb[2], 5, 'f', 5, QChar(' ')).left(8);
    }
    else {
        rStr = tr("R: %0").arg(int(rgb[0]), 5, 10, QChar(' '));
        gStr = tr("G: %0").arg(int(rgb[1]), 5, 10, QChar(' '));
        bStr = tr("B: %0").arg(int(rgb[2]), 5, 10, QChar(' '));
    }
    ui->value_ADU_R->setText(rStr);
    ui->value_ADU_G->setText(gStr);
    ui->value_ADU_B->setText(bStr);
    qreal
            r = rgb[0]!=0?log2(rgb[0]/QuantumRange):-16,
            g = rgb[1]!=0?log2(rgb[1]/QuantumRange):-16,
            b = rgb[2]!=0?log2(rgb[2]/QuantumRange):-16;
    ui->value_EV_R->setText(QString::number(r,'.',2)+tr(" EV"));
    ui->value_EV_G->setText(QString::number(g,'.',2)+tr(" EV"));
    ui->value_EV_B->setText(QString::number(b,'.',2)+tr(" EV"));
    if ( m_tool != ToolROI ) {
        if ( clearStatus )
            ui->statusBar->showMessage("");
        else
            ui->statusBar->showMessage(tr("x:%0, y:%1, R:%2%, G:%3%, B:%4%")
                                       .arg(int(pos.x()),5,10,QChar(' '))
                                       .arg(int(pos.y()),5,10,QChar(' '))
                                       .arg(100.*rgb[0]/QuantumRange, 5, 'f', 2)
                                       .arg(100.*rgb[1]/QuantumRange,5, 'f', 2)
                                       .arg(100.*rgb[2]/QuantumRange, 5, 'f', 2));
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
    m_fullScreenView->setWindowTitle(m_operator->getName());
    emit operatorNameChanged(text);
}

void Visualization::nameReset()
{
    QString text = m_operator->getLocalizedClassIdentifier();
    ui->operatorName->setText(text);
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
    QColor buttonColor(QApplication::palette(static_cast<const QWidget *>(NULL)).color(QPalette::Button));
    QColor green(buttonColor);
    green.setRed(0); green.setBlue(0);
    QColor lightGray(buttonColor);
    int l, r,g,b;
    lightGray.getRgb(&r,&g,&b);
    l = LUMINANCE(r,g,b);
    lightGray.setRgb(l,l,l);


    QTreeWidgetItem *tree_inputs = new QTreeWidgetItem(tree);
    tree_inputs->setText(0, tr("Inputs:"));
    tree_inputs->setFont(0,QFont("Sans", 14));
    tree_inputs->setBackground(0,QBrush(lightGray));
    foreach(OperatorInput *input, m_operator->m_inputs) {
        QTreeWidgetItem *tree_input = new QTreeWidgetItem(tree_inputs);
        tree_input->setText(0, input->name());
        tree_input->setFont(0, QFont("Sans", 12));
        tree_input->setBackground(0, QBrush(green));
        int idx = 0;
        foreach(OperatorOutput *source, input->sources()) {
            QTreeWidgetItem *tree_source = new TreeOutputItem(source, idx, TreeOutputItem::Source, tree_input);
            foreach(Photo photo, source->m_result) {
                if ( !photo.isComplete() )
                    dflCritical(tr("Visualization: source photo is not complete"));
                QString identity = photo.getIdentity();
                identity = identity.split("|").first();
                int count = ++seen[identity];
                if ( count > 1 )
                    identity+=QString("|%0").arg(count-1);
                photo.setIdentity(identity);

                QString treatTag = photo.getTag(TAG_TREAT);
                if ( m_operator->isTagOverrided(identity, TAG_TREAT) )
                    treatTag = m_operator->getTagOverrided(identity, TAG_TREAT);
                TreePhotoItem::PhotoType type = TreePhotoItem::InputEnabled;
                if ( treatTag == TAG_TREAT_DISCARDED )
                    type = TreePhotoItem::InputDisabled;
                else if ( treatTag == TAG_TREAT_REFERENCE )
                    type = TreePhotoItem::InputReference;
                else if ( treatTag == TAG_TREAT_ERROR )
                    type = TreePhotoItem::InputError;
                TreePhotoItem *item = new TreePhotoItem(photo, type, tree_source);
                if ( identity == m_currentPhoto &&
                     source == m_currentOutput ) {
                    item->setSelected(true);
                }
            }
            ++idx;
        }
    }

    QTreeWidgetItem *tree_outputs = new QTreeWidgetItem(tree);
    tree_outputs->setText(0, tr("Outputs:"));
    tree_outputs->setFont(0, QFont("Sans", 14));
    tree_outputs->setBackground(0, QBrush(lightGray));
    int idx = 0;
    foreach(OperatorOutput *output, m_operator->m_outputs) {
        QTreeWidgetItem *tree_output = new TreeOutputItem(output,
                                                          idx,
                                                          m_operator->m_outputStatus[idx] == Operator::OutputEnabled
                                                          ? TreeOutputItem::EnabledSink
                                                          : TreeOutputItem::DisabledSink,
                                                          tree_outputs);
        if (m_operator->m_outputStatus[idx] == Operator::OutputEnabled) {
            foreach(const Photo& photo, output->m_result) {
                if ( !photo.isComplete() )
                    dflCritical(tr("Visualization: output photo is not complete"));
                TreePhotoItem *item = new TreePhotoItem(photo, TreePhotoItem::Output, tree_output);
                if ( photo.getIdentity() == m_currentPhoto &&
                     output == m_currentOutput ) {
                    item->setSelected(true);
                }
            }
        }
        ++idx;
    }
    tree->expandAll();
}

void Visualization::photoSelectionChanged()
{
    QList<QTreeWidgetItem*> items = ui->tree_photos->selectedItems();

    if ( items.count() > 1 ) {
        dflDebug(tr("Invalid photo selection in visualization tree view: sel count %0").arg(items.count()));
        return;
    }

    clearAllTabs();
    foreach(QTreeWidgetItem *item, items ) {
        switch(item->type()) {
        case TreePhotoItem::Type: {
            TreePhotoItem *photoItem = dynamic_cast<TreePhotoItem*>(item);
            m_photoItem = photoItem;
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
    m_photoItem = NULL;

    ui->value_width->setText("0");
    ui->value_height->setText("0");
    updateColorLabels(QPointF(-1,-1));
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
    bool asInput = false;
    QString typeTag = m_photo->getTag(TAG_SCALE);
    if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), TAG_SCALE)) {
        typeTag = m_operator->getTagOverrided(m_photo->getIdentity(), TAG_SCALE);
    }
    if ( (typeTag.isEmpty() || typeTag == TAG_SCALE_LINEAR ) && ui->combo_input->currentIndex() != 0 ) {
        bool state = ui->combo_input->blockSignals(true);
        ui->combo_input->setCurrentIndex(0);
        ui->combo_input->blockSignals(state);
    }
    else if ( typeTag == TAG_SCALE_NONLINEAR && ui->combo_input->currentIndex() != 1 ) {
        bool state = ui->combo_input->blockSignals(true);
        ui->combo_input->setCurrentIndex(1);
        ui->combo_input->blockSignals(state);
        asInput = true;
    }
    else if ( typeTag == TAG_SCALE_HDR && ui->combo_input->currentIndex() != 2 ) {
        bool state = ui->combo_input->blockSignals(true);
        ui->combo_input->setCurrentIndex(2);
        ui->combo_input->blockSignals(state);
    }
    if ( asInput ) {
        bool state = ui->combo_gamma->blockSignals(true);
        ui->combo_gamma->setCurrentIndex(0);
        ui->combo_gamma->blockSignals(state);
    }

    expChanged();
    curveParamsChanged();
    histogramParamsChanged();
    updateTagsTable();
    updateColorLabels(QPointF(-1,-1));
#if 0
    QString geometry = QString::number(m_photo->image().columns()) + " x " + QString::number(m_photo->image().rows());
    ui->value_size->setText(geometry);
#else
    ui->value_width->setText(QString::number(m_photo->image().columns()));
    ui->value_height->setText(QString::number(m_photo->image().rows()));
#endif
    if ( m_photoIsInput ) {
        setInputControlEnabled(true);
    }
    QString roiTag = m_photo->getTag(TAG_ROI);
    if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), TAG_ROI) ) {
        roiTag = m_operator->getTagOverrided(m_photo->getIdentity(), TAG_ROI);
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

    QString treatTag = m_photo->getTag(TAG_TREAT);
    if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), TAG_TREAT)) {
        treatTag = m_operator->getTagOverrided(m_photo->getIdentity(), TAG_TREAT);
    }
    if ( treatTag == TAG_TREAT_REFERENCE && ui->combo_treatment->currentIndex() != 1 ) {
        bool state = ui->combo_treatment->blockSignals(true);
        ui->combo_treatment->setCurrentIndex(1);
        ui->combo_treatment->blockSignals(state);
    }
    else if ( treatTag == TAG_TREAT_DISCARDED && ui->combo_treatment->currentIndex() != 2) {
        bool state = ui->combo_treatment->blockSignals(true);
        ui->combo_treatment->setCurrentIndex(2);
        ui->combo_treatment->blockSignals(state);
    }
    else if ( treatTag == TAG_TREAT_ERROR && ui->combo_treatment->currentData() != 3 ) {
        bool state = ui->combo_treatment->blockSignals(true);
        ui->combo_treatment->setCurrentIndex(3);
        ui->combo_treatment->blockSignals(state);
    }

    reloadPoints();
}

void Visualization::updateTabsWithOutput()
{

}


void Visualization::setInputControlEnabled(bool v)
{
    if (v)
        toolChanged(ui->combo_tool->currentIndex());
    else
        toolChanged(0);
    ui->table_tags->setEnabled(v);
    ui->combo_tool->setEnabled(v);
    ui->combo_treatment->setEnabled(v);
    if ( !m_photo ) ui->combo_treatment->setCurrentIndex(0);
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
        ui->statusBar->showMessage(tr("Selection: x1:%0, y1:%1, x2:%2, y2:%3").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()));
    m_roi_p1 = p1;
    m_roi_p2 = p2;
}

void Visualization::progress(int p, int c)
{
    ui->progressBar->setValue(100.*p/c);
}

bool Visualization::eventFilter(QObject *obj, QEvent *event)
{
    switch(event->type()) {
    case QEvent::Leave: {
        updateColorLabels(QPointF(-1,-1));
        break;
    }
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *me =
                dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        if ( m_tool != ToolROI )
            updateColorLabels(me->scenePos());
        break;
    }
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
                addPoint(me->scenePos(), m_points.count()+1);
                bool evicted = clearPoints(m_tool);
                storePoints();
                if (evicted)
                    reloadPoints();
          }
        }
    }
        break;
    case QEvent::Resize: {
        if (ui->radio_fitVisible->isChecked())
            graphicsViewInteraction->fitVisible();
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
    m_operator->setTagOverride(m_photo->getIdentity(),TAG_ROI, roiTag);
}


void Visualization::reloadPoints()
{
    clearPoints(ToolNone);
    QString pointsTag = m_photo->getTag(TAG_POINTS);
    if ( m_photoIsInput && m_operator->isTagOverrided(m_photo->getIdentity(), TAG_POINTS) ) {
        pointsTag = m_operator->getTagOverrided(m_photo->getIdentity(), TAG_POINTS);
    }
    QStringList coords = pointsTag.split(';');

    for ( int i = 0 ; i < coords.count() ; ++i ) {
        QString &coord = coords[coords.count()-i-1];
        QStringList coordStr = coord.split(',');
        if (coordStr.count() != 2) continue;
        addPoint(QPointF(coordStr[0].toDouble(),coordStr[1].toDouble()), i+1);
    }
}

bool Visualization::clearPoints(Tool tool)
{
    bool evicted = false;
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
        evicted = true;
    }
    return evicted;
}

void Visualization::addPoint(QPointF scenePos, int pointNumber)
{
    VisPoint *point = new VisPoint(scenePos, this, pointNumber);
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
    for ( int i = 0 ; i < m_points.count() ; ++i ) {
        VisPoint *point = m_points[i];
        if ( pointsTag.count() )
            pointsTag += ";";
        point->scenePos();
        pointsTag += QString::number(point->position().x()) +
                "," +
                QString::number(point->position().y());
    }
    if ( pointsTag.count() )
        m_operator->setTagOverride(m_photo->getIdentity(),TAG_POINTS, pointsTag);
    else
        m_operator->resetTagOverride(m_photo->getIdentity(),TAG_POINTS);
}

void Visualization::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int)
{
    if ( item->type() == TreeOutputItem::Type ) {
        TreeOutputItem *outputItem = dynamic_cast<TreeOutputItem*>(item);
        if ( outputItem ) {
            int idx = outputItem->idx();
            TreeOutputItem::Role role = outputItem->role();
            if ( role != TreeOutputItem::Source ) {
                bool newStatus = role == TreeOutputItem::DisabledSink;
                outputItem->setRole(newStatus
                                    ? TreeOutputItem::EnabledSink
                                    : TreeOutputItem::DisabledSink);
                outputItem->setCaption();
                m_operator->setOutputStatus(idx,
                                            newStatus
                                            ? Operator::OutputEnabled
                                            : Operator::OutputDisabled );
                m_operator->setOutOfDate();
            }
        }
    }
}
