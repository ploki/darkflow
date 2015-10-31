#ifndef FILESSELECTION_H
#define FILESSELECTION_H

#include <QDialog>
#include <QStringList>
#include <QString>

namespace Ui {
class FilesSelection;
}

class QStandardItemModel;

class FilesSelection : public QDialog
{
    Q_OBJECT
public:
    explicit FilesSelection(const QString& windowCaption,
                            const QString& dir,
                            const QString& filter,
                            QWidget *parent = 0);
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
    QString m_windowCaption;
    QString m_dir;
    QString m_filter;
};

#endif // FILESSELECTION_H
