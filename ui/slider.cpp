#include "slider.h"
#include "ui_slider.h"


Slider::Slider(const QString &windowCaption,
               Slider::Unit unit,
               Slider::Scale scale,
               qreal min,
               qreal max,
               qreal value,
               bool hardMin,
               bool hardMax,
               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Slider),
    m_unit(unit),
    m_scale(scale),
    m_min(min),
    m_max(max),
    m_value(value),
    m_hardMin(hardMin),
    m_hardMax(hardMax)
{
    ui->setupUi(this);
    setWindowTitle(windowCaption);
    switch(m_unit) {
    default:            qWarning("Unknown unit in Slider");
    case Value:         ui->radio_value->click(); break;
    case Percent:       ui->radio_percent->click(); break;
    case ExposureValue: ui->radio_ev->click(); break;
    case Magnitude:     ui->radio_mag->click(); break;
    }
    switch(m_scale) {
    default:            qWarning("Unknown scale in Slider");
    case Linear:        ui->radio_linear->click(); break;
    case Logarithmic:   ui->radio_logarithmic->click(); break;
    }
    if (m_hardMin)
        ui->dspin_min->setEnabled(false);
    if (m_hardMax)
        ui->dspin_max->setEnabled(false);
}

Slider::~Slider()
{
    delete ui;
}

void Slider::selectValue(bool b)
{
    if (b) m_unit = Value;
}

void Slider::selectPercent(bool b)
{
    if (b) m_unit = Percent;
}

void Slider::selectExposureValue(bool b)
{
    if (b) m_unit = ExposureValue;
}

void Slider::selectMagnitude(bool b)
{
    if (b) m_unit = Magnitude;
}

void Slider::selectLinear(bool b)
{
    if (b) m_scale = Linear;
}

void Slider::selectLogarithmic(bool b)
{
    if (b) m_scale = Logarithmic;
}
