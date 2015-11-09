#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QMainWindow>

namespace Ui {
class Visualization;
}

class Operator;
class OperatorOutput;
class Photo;

class Visualization : public QMainWindow
{
    Q_OBJECT
public:
    explicit Visualization(Operator *op, QWidget *parent = 0);
    ~Visualization();

signals:
    void operatorNameChanged(QString text);

public slots:
    void nameChanged(QString text);
    void operatorUpdated();
    void updateTreeviewPhotos();
    void photoSelectionChanged();

private:
    Ui::Visualization *ui;
    Operator *m_operator;
    OperatorOutput *m_output;
    Photo *m_photo;

    void clearAllTabs();
    void updateTabs();
    void updateTabsWithPhoto();
    void updateTabsWithOutput();

};

#endif // VISUALIZATION_H
