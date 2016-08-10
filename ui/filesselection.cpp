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
#include <QString>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileDialog>

#include "filesselection.h"
#include "ui_filesselection.h"
#include "darkflow.h"

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
    setWindowIcon(QIcon(DF_ICON));
    setWindowFlags(Qt::Tool);
    ui->selectedFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->selectedFiles->setModel(m_list);
}

FilesSelection::~FilesSelection()
{
    delete ui;
    delete m_list;
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

