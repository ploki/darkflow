#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QGuiApplication>
#include <QScreen>
#include <cmath>

#include "console.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "projectproperties.h"
#include "process.h"
#include "processscene.h"
#include "preferences.h"

void MainWindow::showAboutDialog()
{
    aboutDialog->show();
}

void MainWindow::actionExit()
{
    Console::close();
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

void MainWindow::actionPreferences()
{
    preferences->show();
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
        QString filename = QFileDialog::getOpenFileName(this,
                                                        tr("Select a project file"),
                                                        process->projectFile(),
                                                        "Darkflow Project (*.dflow)",
                                                        0, 0);
        if ( !filename.isEmpty()) {
            newProcess();
            process->load(filename);
        }
    }

}

void MainWindow::actionSaveAs()
{
    projectProperties->modifyAndSave(process);
}

void MainWindow::actionConsole()
{
    Console::show();
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
    preferences(0 /* defered because of console */ ),
    projectProperties(new ProjectProperties(this)),
    scene(new ProcessScene(this)),
    process(new Process(scene, this)),
    zoom(0)
{
    Console::init();
    preferences = new Preferences(this);
    ui->setupUi(this);
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    resize( screenSize * 4 / 5);
    move((screenSize.width()-size().width())/2,
         (screenSize.height()-size().height())/2);
    scene->setBackgroundBrush(QColor(0x2e,0x34,0x36));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->installEventFilter(this);
    /* viewport receives first wheel events, it must ignore it to prevent scrolls */
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    newProcess();
}

MainWindow::~MainWindow()
{
    delete process;
    delete scene;
    delete projectProperties;
    delete preferences;
    delete aboutDialog;
    delete ui;
    Console::fini();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::Wheel )
    {
        int delta = dynamic_cast<QWheelEvent*>(event)->delta();
        const int min = -5;
        const int max = 5;
        if(delta > 0)
            zoom+=1;
        else
            zoom-=1;
        zoom=qMax(qMin(zoom,max),min);
        qreal factor = pow(3.,zoom/5.);
        ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        ui->graphicsView->setTransform(QTransform(factor, 0., 0., factor, 0, 0));
        event->accept();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
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
        Console::close();
        preferences->close();
        event->accept();
        scene->clear();
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

