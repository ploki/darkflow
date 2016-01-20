#ifndef GUI_ABOUTDIALOG_H
#define GUI_ABOUTDIALOG_H

#include <QDialog>
#include <QTimer>


namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = 0);

    ~AboutDialog();

private slots:
    void updateUsage();

private:
    Ui::AboutDialog *ui;
    QTimer m_timer;
};

#endif // GUI_ABOUTDIALOG_H
