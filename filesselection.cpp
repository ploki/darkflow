#include <QString>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileDialog>

#include "filesselection.h"
#include "ui_filesselection.h"

FilesSelection::FilesSelection(const QString& windowCaption,
                               const QString& dir,
                               const QString& filter,
                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilesSelection),
    m_list(new QStandardItemModel),
    m_windowCaption(windowCaption),
    m_dir(dir),
    m_filter(filter)
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
                                                     m_windowCaption,
                                                     m_dir,
                                                     m_filter,
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

