#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "darkflow.h"
#include "ports.h"
#include "darkflow-version.iss"

#include <Magick++.h>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    m_timer()
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    ui->pixmapWidget->setPixmap(QPixmap(DF_ICON));
    ui->version->setText("Dark Flow version " Version " " DF_ARCH);
    m_timer.setInterval(500);
    connect(&m_timer,SIGNAL(timeout()), this, SLOT(updateUsage()));
    m_timer.start();
    adjustSize();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::updateUsage()
{
    if ( !isVisible() ) return;
    MagickCore::MagickSizeType area = MagickCore::GetMagickResource(MagickCore::AreaResource);
    MagickCore::MagickSizeType memory = MagickCore::GetMagickResource(MagickCore::MemoryResource);
    MagickCore::MagickSizeType map = MagickCore::GetMagickResource(MagickCore::MapResource);
    MagickCore::MagickSizeType disk = MagickCore::GetMagickResource(MagickCore::DiskResource);
    ui->lineArea->setText(tr("%0 GB").arg(double(area)/(1<<30), 0, 'f', 3));
    ui->lineMemory->setText(tr("%0 GB").arg(double(memory)/(1<<30), 0, 'f', 3));
    ui->lineMap->setText(tr("%0 GB").arg(double(map)/(1<<30), 0, 'f', 3));
    ui->lineDisk->setText(tr("%0 GB").arg(double(disk)/(1<<30), 0, 'f', 3));
}
