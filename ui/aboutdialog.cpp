#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "slider.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    Slider *sl = new Slider("plop!", Slider::ExposureValue,
                            Slider::Logarithmic, Slider::Real,
               1, 1<<8, 1, 1, 65535, Slider::FilterAll);
    sl->show();
    sl->raise();

    sl = new Slider("plop Integer !", Slider::Percent,
                                Slider::Linear, Slider::Integer,
                   -100, 100, 14, -800, 800, Slider::FilterPercent);
        sl->show();
        sl->raise();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
