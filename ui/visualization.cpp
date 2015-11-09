#include <QTreeWidget>
#include <QTreeWidgetItem>
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
    m_photo(NULL)
{
    ui->setupUi(this);
    ui->operatorName->setText(m_operator->getName());
    ui->operatorClass->setText(m_operator->getClassIdentifier());
    setWindowTitle(m_operator->getName());
    connect(ui->operatorName, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->tree_photos, SIGNAL(itemSelectionChanged()), this, SLOT(photoSelectionChanged()));
    updateTreeviewPhotos();
}

Visualization::~Visualization()
{
    delete ui;
}

void Visualization::nameChanged(QString text)
{
    setWindowTitle(text);
    emit operatorNameChanged(text);
}

void Visualization::operatorUpdated()
{
    updateTreeviewPhotos();
    this->raise();
}

void Visualization::updateTreeviewPhotos()
{
    QTreeWidget *tree = ui->tree_photos;
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
                new TreePhotoItem(photo, tree_source);
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
            new TreePhotoItem(photo, tree_output);
        }
    }

    tree->expandAll();
}

void Visualization::photoSelectionChanged()
{
    QList<QTreeWidgetItem*> items = ui->tree_photos->selectedItems();

    if ( items.count() > 1 )
        qWarning("Invalid photo selection in visualization tree view");

    clearAllTabs();
    foreach(QTreeWidgetItem *item, items ) {
        switch(item->type()) {
        case TreePhotoItem::Type:
            m_photo = dynamic_cast<TreePhotoItem*>(item)->photo();
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
    ui->widget_visualization->setPixmap(m_photo->toPixmap(2.4, 0.00304L,1));
}

void Visualization::updateTabsWithOutput()
{

}
