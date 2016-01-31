/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include "slider.h"
#include "ui_slider.h"
#include "console.h"
#include "darkflow.h"
#include "mainwindow.h"

#include <cmath>


Slider::Slider(const QString &windowCaption,
               Slider::Unit unit,
               Slider::Scale scale,
               Slider::NumberSet numberSet,
               qreal min,
               qreal max,
               qreal value,
               qreal hardMin,
               qreal hardMax,
               uint parametersFilter,
               QWidget *parent) :
    QDialog(parent ? parent : dflMainWindow ),
    ui(new Ui::Slider),
    m_unit(unit),
    m_scale(scale),
    m_numberSet(numberSet),
    m_min(min),
    m_max(max),
    m_value(value),
    m_hardMin(hardMin),
    m_hardMax(hardMax),
    m_range(0),
    m_pvDecimals(-1)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    setWindowTitle(windowCaption);
    m_range = ui->slider_value->maximum() - ui->slider_value->minimum();

    if ( m_hardMin > m_min ) {
        dflWarning(tr("Slider: hardMin > min"));
        m_min = m_hardMin;
    }
    if ( m_hardMax < m_max ) {
        dflWarning(tr("Slider: hardMax < max"));
        m_max = m_hardMax;
    }

    int md = log10(1./m_hardMax);
    int Md = log10(1./m_hardMin);
    m_pvDecimals = qMax(md,Md)+1;

    if ( m_hardMin <= 0. && m_hardMax >= 0 ) {
        ui->radio_logarithmic->setEnabled(false);
        ui->radio_ev->setEnabled(false);
        ui->radio_mag->setEnabled(false);
        if ( m_scale == Logarithmic ) {
            dflWarning(tr("Slider: wrong scale for +/- value"));
            m_scale = Linear;
        }
        if ( m_unit == ExposureValue || m_unit == Magnitude ) {
            dflWarning(tr("Slider: wrong unit selected for linear scale"));
            m_unit = Value;
        }
        ui->group_scale->hide();
    }
    clickUnit(m_unit);
    clickScale(m_scale);

    if ( !(parametersFilter & (Value|Percent|ExposureValue|Magnitude))) {
        ui->group_unit->hide();
    }
    else {
        if ( !(parametersFilter & Value) )
            ui->radio_value->setEnabled(false);
        if ( !(parametersFilter & Percent) )
            ui->radio_percent->setEnabled(false);
        if ( !(parametersFilter & ExposureValue) )
            ui->radio_ev->setEnabled(false);
        if ( !(parametersFilter & Magnitude) )
            ui->radio_mag->setEnabled(false);
        if ( (ui->radio_value->isEnabled()?1:0) +
             (ui->radio_percent->isEnabled()?1:0) +
             (ui->radio_ev->isEnabled()?1:0) +
             (ui->radio_mag->isEnabled()?1:0) == 1)
            ui->group_unit->hide();
    }
    if ( !(parametersFilter & (Linear|Logarithmic))) {
        ui->group_scale->hide();
    }
    else {
        if ( !(parametersFilter & Linear) )
            ui->radio_linear->setEnabled(false);
        if ( !(parametersFilter & Logarithmic) )
            ui->radio_logarithmic->setEnabled(false);
        if ( (ui->radio_linear->isEnabled()?1:0) +
             (ui->radio_logarithmic->isEnabled()?1:0) == 1)
            ui->group_scale->hide();
    }

    this->adjustSize();

    setSlider();
    updateInputs();
}

Slider::~Slider()
{
    delete ui;
}

void Slider::updateInputs()
{
    bool stateMin, stateMax, stateValue;

    stateMin = ui->dspin_min->blockSignals(true);
    stateMax = ui->dspin_max->blockSignals(true);
    stateValue = ui->dspin_value->blockSignals(true);

    ui->dspin_min->setMinimum(toUnit(m_hardMin));
    ui->dspin_min->setMaximum(toUnit(m_value));
    ui->dspin_min->setValue(toUnit(m_min));

    ui->dspin_max->setMinimum(toUnit(m_value));
    ui->dspin_max->setMaximum(toUnit(m_hardMax));
    ui->dspin_max->setValue(toUnit(m_max));

    ui->dspin_value->setMinimum(toUnit(m_min));
    ui->dspin_value->setMaximum(toUnit(m_max));
    ui->dspin_value->setValue(toUnit(m_value));

    int decimals;
    switch (m_numberSet) {
    default:
    case Real: {
        switch (m_unit) {
        default:
            decimals = 2;
            break;
        case Value:
            decimals = m_pvDecimals;
            break;
        case Percent:
            decimals = qMax(0, m_pvDecimals-2);
            break;
        }
        break;
    }
    case Integer: {
        decimals = 0;
        break;
    }
    }
    ui->dspin_min->setDecimals(decimals);
    ui->dspin_max->setDecimals(decimals);
    ui->dspin_value->setDecimals(decimals);
    ui->dspin_min->blockSignals(stateMin);
    ui->dspin_max->blockSignals(stateMax);
    ui->dspin_value->blockSignals(stateValue);
    emit updated();
}

void Slider::selectValue(bool b)
{
    if (b) {
        setUnit(Value);
        setSlider();
        updateInputs();
    }
}

void Slider::selectPercent(bool b)
{
    if (b) {
        setUnit(Percent);
        setSlider();
        updateInputs();
    }
}

void Slider::selectExposureValue(bool b)
{
    if (b) {
        setUnit(ExposureValue);
        setSlider();
        updateInputs();
    }
}

void Slider::selectMagnitude(bool b)
{
    if (b) {
        setUnit(Magnitude);
        setSlider();
        updateInputs();
    }
}

void Slider::selectLinear(bool b)
{
    if (b) {
        m_scale = Linear;
        setSlider();
        updateInputs();
    }
}

void Slider::selectLogarithmic(bool b)
{
    if (b) {
        m_scale = Logarithmic;
        setSlider();
        updateInputs();
    }
}

void Slider::changeMin(double v)
{
    m_min = fromUnit(v);
    setSlider();
    updateInputs();
}

void Slider::changeMax(double v)
{
    m_max = fromUnit(v);
    setSlider();
    updateInputs();
}

void Slider::changeValue(double v)
{
    m_value = fromUnit(v);
    setSlider();
    updateInputs();
}

void Slider::sliderChanged()
{
    m_value = getSlider();
    updateInputs();
}

void Slider::loadValues(Slider::Unit unit, Slider::Scale scale, qreal min, qreal max, qreal value)
{
    clickUnit(unit);
    clickScale(scale);
    m_min = min;
    m_max = max;
    m_value = value;
    setSlider();
    updateInputs();
}

void Slider::setSlider()
{
    double v = m_value;
    qreal sliderValue;
    switch (m_scale) {
    case Logarithmic:
        v = log2(v);
        sliderValue = (v-log2(m_min))/log2(m_max/m_min)*m_range;
        break;
    default:
    case Linear:
        sliderValue = (v-m_min)/(m_max-m_min)*m_range;
    }
    bool state;
    state = ui->slider_value->blockSignals(true);
    ui->slider_value->setValue(int(sliderValue));
    ui->slider_value->blockSignals(state);
}

double Slider::getSlider() {
    qreal sliderValue = ui->slider_value->value();
    qreal v;
    switch (m_scale) {
    case Logarithmic:
        v = log2(m_max/m_min)*sliderValue/m_range+log2(m_min);
        v = pow(2.L,v);
        break;
    default:
    case Linear:
        v = (m_max-m_min)*sliderValue/m_range+m_min;
    }
    return v;
}

qreal Slider::fromValue_(qreal v)
{
    return v;
}

qreal Slider::fromPercent_(qreal v)
{
    return v/100.L;
}

qreal Slider::fromExposureValue_(qreal v)
{
    return pow(2.L,v);
}

qreal Slider::fromMagnitude_(qreal v)
{
    return pow(MAGNITUDE_POW,v);
}

qreal Slider::fromUnit(qreal v)
{
    switch(m_unit) {
    default:
    case Value:         return fromValue_(v);
    case Percent:       return fromPercent_(v);
    case ExposureValue: return fromExposureValue_(v);
    case Magnitude:     return fromMagnitude_(v);
    }
}

qreal Slider::toValue_(qreal v)
{
    return v;

}

qreal Slider::toPercent_(qreal v)
{
    return v*100.L;
}

qreal Slider::toExposureValue_(qreal v)
{
    return log2(v);
}

qreal Slider::toMagnitude_(qreal v)
{
    return log2(v)/log2(MAGNITUDE_POW);
}

qreal Slider::toUnit(qreal v)
{
    switch(m_unit) {
    default:
    case Value: return toValue_(v);
    case Percent: return toPercent_(v);
    case ExposureValue: return toExposureValue_(v);
    case Magnitude: return toMagnitude_(v);
    }
}

void Slider::setUnit(Slider::Unit u)
{
    m_unit = u;
    QString unit = " " + unitToLocalizedString(m_unit);
    ui->dspin_min->setSuffix(unit);
    ui->dspin_max->setSuffix(unit);
    ui->dspin_value->setSuffix(unit);
    double singleStep;
    switch(u) {
    default:
    case Value:
        singleStep = .1L;
        break;
    case Percent:
        singleStep = 10.L;
        break;
    case ExposureValue:
        singleStep = .25L;
        break;
    case Magnitude:
        singleStep = .25L;
        break;
    }
    ui->dspin_min->setSingleStep(singleStep);
    ui->dspin_max->setSingleStep(singleStep);
    ui->dspin_value->setSingleStep(singleStep);
}

void Slider::clickUnit(Slider::Unit u)
{
    switch(u) {
    default:            dflWarning("Slider: Unknown unit");
    case Value:         ui->radio_value->click(); break;
    case Percent:       ui->radio_percent->click(); break;
    case ExposureValue: ui->radio_ev->click(); break;
    case Magnitude:     ui->radio_mag->click(); break;
    }
}

void Slider::clickScale(Slider::Scale s)
{
    switch(s) {
    default:            dflWarning("Slider: Unknown scale");
    case Linear:        ui->radio_linear->click(); break;
    case Logarithmic:   ui->radio_logarithmic->click(); break;
    }
}

const char *Slider::unitToString(Slider::Unit unit)
{
    switch(unit) {
    default:
        dflWarning(tr("Slider: Unknown unit"));
    case Value: return QT_TR_NOOP("");
    case Percent: return QT_TR_NOOP("%");
    case ExposureValue: return QT_TR_NOOP("EV");
    case Magnitude: return QT_TR_NOOP("M");
    }
}
QString Slider::unitToLocalizedString(Slider::Unit unit)
{
    return tr(unitToString(unit));}

Slider::Unit Slider::unitFromString(const QString &unit)
{
    if ( unit == QT_TR_NOOP("") )
        return Value;
    else if ( unit == QT_TR_NOOP("%") )
        return Percent;
    else if ( unit == QT_TR_NOOP("EV") )
        return ExposureValue;
    else if ( unit == QT_TR_NOOP("M") )
        return Magnitude;
    else {
        dflWarning(tr("Slider: Unknown unit"));
        return Value;
    }
}

QString Slider::scaleToString(Slider::Scale scale)
{
    switch(scale) {
    default:
        dflWarning(tr("Slider: Unknown scale"));
    case Linear:
        return QT_TR_NOOP("Linear");
    case Logarithmic:
        return QT_TR_NOOP("Logarithmic");
    }
}

Slider::Scale Slider::scaleFromString(const QString &scale)
{
    if (scale == QT_TR_NOOP("Linear"))
        return Linear;
    else if (scale == QT_TR_NOOP("Logarithmic"))
        return Logarithmic;
    else {
        dflWarning(tr("Slider: Unknown scale"));
        return Linear;
    }
}

qreal Slider::getValue() const
{
    return m_value;
}

QString Slider::currentValue() const
{
    return ui->dspin_value->text();
}

qreal Slider::getMax() const
{
    return m_max;
}

qreal Slider::getMin() const
{
    return m_min;
}

Slider::Scale Slider::getScale() const
{
    return m_scale;
}

Slider::Unit Slider::getUnit() const
{
    return m_unit;
}
