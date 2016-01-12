#include <QFileDialog>
#include <QMessageBox>
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
                                                    "Darkflow Project (*.dflow)",
                                                    0, 0);
    if ( filename.isEmpty())
        return;
    if ( !filename.endsWith(".dflow",Qt::CaseInsensitive) )
        filename+=".dflow";
    ui->project_file->setText(filename);
}

void ProjectProperties::selectBaseDirectory()
{
    QString baseDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                          ui->valueBaseDir->text(),
                                                          QFileDialog::ShowDirsOnly);
    if ( !baseDir.isEmpty())
        ui->valueBaseDir->setText(baseDir);

}

void ProjectProperties::accept()
{
    if (m_process) {
        m_process->setProjectName(ui->project_name->text());
        m_process->setProjectFile(ui->project_file->text());
        m_process->setNotes(ui->notes->toPlainText());
        m_process->setBaseDirectory(ui->valueBaseDir->text());
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


