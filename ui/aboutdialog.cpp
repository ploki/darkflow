#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "darkflow.h"
#include "ports.h"
#include "darkflow-version.iss"


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    ui->pixmapWidget->setPixmap(QPixmap(DF_ICON));
    ui->version->setText("Dark Flow version " Version " " DF_ARCH);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
