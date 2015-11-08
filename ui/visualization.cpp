#include "visualization.h"
#include "ui_visualization.h"
#include "operator.h"

Visualization::Visualization(Operator *op, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Visualization),
    m_operator(op)
{
    ui->setupUi(this);
    ui->operatorName->setText(m_operator->getName());
    connect(ui->operatorName, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
}

Visualization::~Visualization()
{
    delete ui;
}

void Visualization::nameChanged(QString text)
{
    emit operatorNameChanged(text);
}
