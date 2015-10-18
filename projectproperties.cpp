#include <QFileDialog>
#include <QMessageBox>
#include "projectproperties.h"
#include "ui_projectproperties.h"
#include "process.h"

ProjectProperties::ProjectProperties(QWidget *parent) :
    QDialog(parent),
    ui( new Ui::ProjectProperties),
    m_process(NULL),
    m_andSave(false)
{
    ui->setupUi(this);
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
        ui->output_directory->setText(process->outputDirectory());
        ui->temporary_directory->setText(process->temporaryDirectory());
    }
    this->show();
}

ProjectProperties::~ProjectProperties()
{
    delete ui;
}

void ProjectProperties::selectProjectFile()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Select a project file"),
                                                    ui->project_file->text(),
                                                    "Darkflow Project (*.dflow)",
                                                    0, 0);
    if ( !filename.isEmpty())
        ui->project_file->setText(filename);
}

void ProjectProperties::selectOutputDirectory()
{
    QString outputDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                          ui->output_directory->text(),
                                                          QFileDialog::ShowDirsOnly);
    if ( !outputDir.isEmpty())
        ui->output_directory->setText(outputDir);

}

void ProjectProperties::selectTemporaryDirectory()
{
    QString tmpDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                       ui->temporary_directory->text(),
                                                       QFileDialog::ShowDirsOnly);
    if ( !tmpDir.isEmpty())
        ui->temporary_directory->setText(tmpDir);
}

void ProjectProperties::accept()
{
    if (m_process) {
        m_process->setProjectName(ui->project_name->text());
        m_process->setProjectFile(ui->project_file->text());
        m_process->setNotes(ui->notes->toPlainText());
        m_process->setOutputDirectory(ui->output_directory->text());
        m_process->setTemporaryDirectory(ui->temporary_directory->text());
        if (m_andSave) {
            if ( m_process->projectFile().isEmpty())
                QMessageBox::warning( this, this->objectName(),
                                      tr("Project filename not defined\n"));
            m_process->save();
        }
        m_process = NULL;
    }
    this->close();
}


