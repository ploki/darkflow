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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class AboutDialog;
class Preferences;
class ProjectProperties;
class Process;
class ProcessScene;
class GraphicsViewInteraction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setSceneBackgroundBrush(const QColor& color);

    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void showAboutDialog();
    void actionExit();
    void actionNewProject();
    void actionProjectProperties();
    void actionPreferences();
    void actionSave();
    void actionLoad();
    void actionSaveAs();
    void actionConsole();
    void actionOnlineDocumentation();
    void load(const QString& filename);

private slots:
    void processStateChanged();

private:
    Ui::MainWindow *ui;
    AboutDialog *aboutDialog;
    ProjectProperties *projectProperties;
    ProcessScene *scene;
    Process *process;
    bool zoomKey;
    qreal totalScaleFactor;
    qreal lastGestureFactor;
    GraphicsViewInteraction *graphicsViewInteraction;
    void closeEvent(QCloseEvent *event);
    void newProcess();

};

extern MainWindow *dflMainWindow;

#endif // MAINWINDOW_H
