#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QMainWindow>

namespace Ui {
class Visualization;
}

class Operator;

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

private:
    Ui::Visualization *ui;
    Operator *m_operator;
};

#endif // VISUALIZATION_H
