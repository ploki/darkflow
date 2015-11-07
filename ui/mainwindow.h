#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class AboutDialog;
class ProjectProperties;
class Process;
class ProcessScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *, QEvent *);

public slots:
    void showAboutDialog();
    void actionExit();
    void actionNewProject();
    void actionProjectProperties();
    void actionSave();
    void actionLoad();
    void actionSaveAs();

private slots:
    void processStateChanged();

private:
    Ui::MainWindow *ui;
    AboutDialog *aboutDialog;
    ProjectProperties *projectProperties;
    ProcessScene *scene;
    Process *process;
    int zoom;

    void closeEvent(QCloseEvent *event);
    void newProcess();

};

#endif // MAINWINDOW_H
