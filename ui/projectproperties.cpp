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
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include "projectproperties.h"
#include "ui_projectproperties.h"
#include "process.h"
#include "darkflow.h"

ProjectProperties::ProjectProperties(QWidget *parent) :
    QDialog(parent),
    ui( new Ui::ProjectProperties),
    m_process(NULL),
    m_andSave(false)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
}

void ProjectProperties::modify(Process *process)
{
    modifyInternal(process, false);
}

void ProjectProperties::modifyAndSave(Process *process)
{
    modifyInternal(process, true);
}

void ProjectProperties::modifyInternal(Process *process, bool andSave)
{
    m_andSave = andSave;
    if (process) {
        m_process = process;
        ui->project_name->setText(process->projectName());
        ui->notes->setPlainText(process->notes());
        ui->project_file->setText(process->projectFile());
        ui->valueBaseDir->setText(process->baseDirectory());
    }
    this->show();
}

ProjectProperties::~ProjectProperties()
{
    delete ui;
}

void ProjectProperties::selectProjectFile()
{
    QString base = ui->project_file->text();
    if (base.isEmpty())
        base = ui->valueBaseDir->text();
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Select a project file"),
                                                    base,
                                                    tr("Darkflow Project (*.dflow)"),
                                                    0, 0);
    if ( filename.isEmpty())
        return;
    if ( !filename.endsWith(".dflow",Qt::CaseInsensitive) )
        filename+=".dflow";
    ui->project_file->setText(filename);
}

void ProjectProperties::selectBaseDirectory()
{
    QString baseDir = QFileDialog::getExistingDirectory(this, tr("Select Base Directory"),
                                                          ui->valueBaseDir->text(),
                                                          QFileDialog::ShowDirsOnly);
    if ( !baseDir.isEmpty())
        ui->valueBaseDir->setText(baseDir);

}

void ProjectProperties::accept()
{
    QString projectFile = ui->project_file->text();
    QString baseDir = ui->valueBaseDir->text();
    if (m_process) {
        if ( baseDir.isEmpty() ) {
            QFileInfo finfo(projectFile);
            baseDir = finfo.absoluteDir().absolutePath();
        }
        m_process->setProjectName(ui->project_name->text());
        m_process->setProjectFile(projectFile);
        m_process->setNotes(ui->notes->toPlainText());
        m_process->setBaseDirectory(baseDir);
        if (m_andSave) {
            if ( m_process->projectFile().isEmpty())
                QMessageBox::warning( this, this->objectName(),
                                      tr("Project filename not defined"));
            m_process->save();
        }
        m_process = NULL;
    }
    this->close();
}


