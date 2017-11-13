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
#ifndef SLIDER_H
#define SLIDER_H

#include <QDialog>

#define MAGNITUDE_POW (2.512L)

namespace Ui {
class Slider;
}

class QAbstractButton;

class Slider : public QDialog
{
    Q_OBJECT
public:
    typedef enum {
        Value         = (1<<0),
        Percent       = (1<<1),
        ExposureValue = (1<<2),
        Magnitude     = (1<<3)
    } Unit;
    typedef enum {
        Linear        = (1<<4),
        Logarithmic   = (1<<5)
    } Scale;
    typedef enum {
        Integer,
        Real
    } NumberSet;
    enum {
        FilterNothing = 0,
        FilterAll = -1,
        FilterExposureFromOne = Logarithmic|ExposureValue|Magnitude|Percent,
        FilterExposure = Logarithmic|ExposureValue|Magnitude,
        FilterPixels = Linear|Value,
        FilterPercent = Linear|Percent,
        FilterLogarithmic = -1&(~Linear),
        FilterLinear = -1&(~(Logarithmic|ExposureValue|Magnitude))
    };
    explicit Slider(const QString& windowCaption,
                    Unit unit,
                    Scale scale,
                    NumberSet numberSet,
                    qreal min,
                    qreal max,
                    qreal value,
                    qreal hardMin,
                    qreal hardMax,
                    uint parametersFilter= FilterAll,
                    QWidget *parent = 0);
    ~Slider();

    Unit getUnit() const;

    Scale getScale() const;

    qreal getMin() const;

    qreal getMax() const;

    qreal getValue() const;

    QString currentValue() const;

public slots:
    void selectValue(bool);
    void selectPercent(bool);
    void selectExposureValue(bool);
    void selectMagnitude(bool);
    void selectLinear(bool);
    void selectLogarithmic(bool);

    void changeMin(double);
    void changeMax(double);
    void changeValue(double);

    void sliderChanged();

    void loadValues(Unit unit,
                    Scale scale,
                    qreal min,
                    qreal max,
                    qreal value);

    static const char* unitToString(Unit unit);
    static QString unitToLocalizedString(Unit unit);
    static Unit unitFromString(const QString& unit);
    static QString scaleToString(Scale scale);
    static Scale scaleFromString(const QString& scale);

signals:
    void updated();


private:
    Ui::Slider *ui;
    Unit m_unit;
    Scale m_scale;
    NumberSet m_numberSet;
    qreal m_min;
    qreal m_max;
    qreal m_value;
    qreal m_hardMin;
    qreal m_hardMax;
    qreal m_range;
    int m_pvDecimals;

    void setSlider();
    double getSlider();

    void updateInputs();

    qreal fromValue_(qreal v);
    qreal fromPercent_(qreal v);
    qreal fromExposureValue_(qreal v);
    qreal fromMagnitude_(qreal v);
    qreal toValue_(qreal v);
    qreal toPercent_(qreal v);
    qreal toExposureValue_(qreal v);
    qreal toMagnitude_(qreal v);

    qreal fromUnit(qreal v);
    qreal toUnit(qreal v);
    void setUnit(Slider::Unit u);
    void clickUnit(Slider::Unit u);
    void clickScale(Slider::Scale s);
};

#endif // SLIDER_H
