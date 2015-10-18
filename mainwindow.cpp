#include <QMessageBox>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "projectproperties.h"
#include "process.h"
#include "processscene.h"

void MainWindow::showAboutDialog()
{
    aboutDialog->show();
}

void MainWindow::actionExit()
{
    this->close();
}

void MainWindow::actionNewProject()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;

    if ( process->dirty() )
        resBtn = QMessageBox::question( this, this->objectName(),
                                        tr("Are you sure?\n"),
                                        QMessageBox::No |
                                        QMessageBox::Yes,
                                        QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {
        newProcess();
        projectProperties->modify(process);
    }
}

void MainWindow::actionProjectProperties()
{
    projectProperties->modify(process);
}

void MainWindow::actionSave()
{
    if ( process->projectFile().isEmpty())
        QMessageBox::warning( this, this->objectName(),
                              tr("Project filename not defined\n"));

    else
        process->save();
}

void MainWindow::actionLoad()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    if ( process->dirty() )
        resBtn = QMessageBox::question( this, this->objectName(),
                                        tr("Are you sure?\n"),
                                        QMessageBox::No |
                                        QMessageBox::Yes,
                                        QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {
        newProcess();
        QString filename = QFileDialog::getOpenFileName(this,
                                                        tr("Select a project file"),
                                                        process->projectFile(),
                                                        "Darkflow Project (*.dflow)",
                                                        0, 0);
        if ( !filename.isEmpty())
            process->open(filename);
    }

}

void MainWindow::actionSaveAs()
{
    projectProperties->modifyAndSave(process);
}

void MainWindow::processStateChanged()
{
    QString title = "DarkFlow";
    if (process->dirty())
        title += " * ";
    else
        title += " - ";
    if ( !process->projectName().isEmpty() )
        title += process->projectName();
    else if ( !process->projectName().isEmpty() )
        title += process->projectFile();
    else
        title += "[New Project]";
    this->setWindowTitle(title);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    aboutDialog(new AboutDialog(this)),
    projectProperties(new ProjectProperties(this)),
    scene(new ProcessScene(this)),
    process(new Process(scene, this))
{
    ui->setupUi(this);
    scene->setBackgroundBrush(QColor(0x2e,0x34,0x36));
    ui->graphicsView->setScene(scene);
    newProcess();
}

MainWindow::~MainWindow()
{
    delete process;
    delete scene;
    delete projectProperties;
    delete aboutDialog;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;

    if ( process->dirty() )
        resBtn = QMessageBox::question( this, this->objectName(),
                                        tr("Are you sure?\n"),
                                        QMessageBox::No |
                                        QMessageBox::Yes,
                                        QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        if (aboutDialog->isVisible())
            aboutDialog->close();
        event->accept();
    }
}

void MainWindow::newProcess()
{
    if (process) {
        disconnect(process, SIGNAL(stateChanged()), this, SLOT(processStateChanged()));
        delete process;
    }
    process = new Process(scene);
    process->reset();
    connect(process, SIGNAL(stateChanged()), this, SLOT(processStateChanged()));
    process->setDirty(false);
    processStateChanged();
}

