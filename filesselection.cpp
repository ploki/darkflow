#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileDialog>

#include "filesselection.h"
#include "ui_filesselection.h"

FilesSelection::FilesSelection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilesSelection),
    m_list(new QStandardItemModel)
{
    ui->setupUi(this);
    ui->selectedFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->selectedFiles->setModel(m_list);
}

FilesSelection::~FilesSelection()
{
    delete ui;
}

QStringList FilesSelection::getSelection() const
{
    QStringList selection;
    int s = m_list->rowCount();
    for (int i = 0 ; i < s ; ++i )
    {
        selection.push_back(m_list->index(i,0).data().toString());
    }
    return selection;
}

void FilesSelection::setSelection(const QStringList &selection)
{
    foreach(QString file, selection) {
        int pos = m_list->rowCount();
        QStandardItem *item = new QStandardItem(file);
        m_list->insertRow(pos);
        m_list->setItem(pos,item);
    }

}

void FilesSelection::clearSelection()
{
    m_list->clear();
}

void FilesSelection::addClicked()
{
    QStringList list = QFileDialog::getOpenFileNames(this,
                                                     tr("Select files to insert"),
                                                     "/home/guigui",
                                                     "RAW Images (*.nef *.cr2 *.dng *.mef *.3fr *.raf *.x3f *.pef *.arw *.nrw);;"
                                                     "FITS Images (*.fits *.fit);;"
                                                     "TIFF Images (*.tif *.tiff);;"
                                                     "All Files (*.*)",
                                                     0, 0);
    setSelection(list);
}

void FilesSelection::removeClicked()
{
    ui->selectedFiles->setUpdatesEnabled(false);
    QModelIndexList indexes = ui->selectedFiles->selectionModel()->selectedIndexes();
    qSort(indexes);
    for (int i = indexes.count() - 1; i > -1; --i)
      m_list->removeRow(indexes.at(i).row());
    ui->selectedFiles->setUpdatesEnabled(true);
}

void FilesSelection::clearClicked()
{
    m_list->clear();
}

