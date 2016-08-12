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
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopServices>
#include <QUrl>
#include <cmath>

#include "darkflow.h"
#include "console.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "projectproperties.h"
#include "process.h"
#include "processscene.h"
#include "preferences.h"
#include "graphicsviewinteraction.h"

MainWindow *dflMainWindow = NULL;

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
        graphicsViewInteraction->zoomSet(1);
        ui->graphicsView->centerOn(0, 0);
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
                                                        preferences->baseDir(),
                                                        "Darkflow Project (*.dflow)",
                                                        0, 0);
        if ( !filename.isEmpty()) {
            load(filename);
        }
    }

}

bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent*>(event);
        QMessageBox::StandardButton resBtn = QMessageBox::Yes;
        if ( process->dirty() )
            resBtn = QMessageBox::question( this, this->objectName(),
                                            tr("Are you sure?\n"),
                                            QMessageBox::No |
                                            QMessageBox::Yes,
                                            QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes) {
            if ( !openEvent->file().isEmpty()) {
                load(openEvent->file());
                event->accept();
                return true;
            }
        }
    }
    return false;
}


void MainWindow::actionSaveAs()
{
    projectProperties->modifyAndSave(process);
}

void MainWindow::actionConsole()
{
    Console::show();
}

void MainWindow::actionOnlineDocumentation()
{
    QDesktopServices::openUrl(QUrl("http://darkflow.org/redirect/"));
}

void MainWindow::load(const QString &filename)
{
    newProcess();
    process->load(filename);
    graphicsViewInteraction->fitVisible();
}

void MainWindow::processStateChanged()
{
    QString title = tr("DarkFlow");
    if (process->dirty())
        title += " * ";
    else
        title += " - ";
    if ( !process->projectName().isEmpty() )
        title += process->projectName();
    else if ( !process->projectName().isEmpty() )
        title += process->projectFile();
    else
        title += tr("[New Project]");
    this->setWindowTitle(title);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    aboutDialog(new AboutDialog(this)),
    projectProperties(new ProjectProperties(this)),
    scene(new ProcessScene(this)),
    process(0 /* postponed because of preferences */),
    zoomKey(false),
    totalScaleFactor(1),
    lastGestureFactor(1),
    graphicsViewInteraction(0)
{
    Console::init();
    preferences = new Preferences(this);
    process = new Process(scene, this);
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    QApplication::instance()->installEventFilter(this);
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    resize( screenSize * 4 / 5);
    move((screenSize.width()-size().width())/2,
         (screenSize.height()-size().height())/2);
    setSceneBackgroundBrush(preferences->color(QPalette::AlternateBase));
    scene->setSceneRect(QRectF(-32768,-32768,65536,65536));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsViewInteraction = new GraphicsViewInteraction(ui->graphicsView, this);
    ui->graphicsView->centerOn(0,0);
    graphicsViewInteraction->zoomSet(1);
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

void MainWindow::setSceneBackgroundBrush(const QColor &color)
{
    scene->setBackgroundBrush(color);
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

