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
