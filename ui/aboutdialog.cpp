#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "darkflow.h"


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    ui->pixmapWidget->setPixmap(QPixmap(DF_ICON));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
