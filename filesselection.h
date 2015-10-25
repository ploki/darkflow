#ifndef FILESSELECTION_H
#define FILESSELECTION_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class FilesSelection;
}

class QStandardItemModel;

class FilesSelection : public QDialog
{
    Q_OBJECT
public:
    explicit FilesSelection(QWidget *parent = 0);
    ~FilesSelection();

    QStringList getSelection() const;
    void setSelection(const QStringList& selection);
    void clearSelection();

public slots:
    void addClicked();
    void removeClicked();
    void clearClicked();

private:
    Ui::FilesSelection *ui;
    QStandardItemModel *m_list;
};

#endif // FILESSELECTION_H
