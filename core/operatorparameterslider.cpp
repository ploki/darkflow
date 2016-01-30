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
#include "operatorparameterslider.h"
#include "slider.h"
#include "console.h"

OperatorParameterSlider::OperatorParameterSlider(const QString &name,
                                                 const QString &caption,
                                                 const QString &windowCaption,
                                                 Slider::Unit unit,
                                                 Slider::Scale scale,
                                                 Slider::NumberSet numberSet,
                                                 qreal min_,
                                                 qreal max_,
                                                 qreal value,
                                                 qreal hardMin,
                                                 qreal hardMax,
                                                 uint parametersFilter,
                                                 Operator *op) :
    OperatorParameter(name, caption, op),
    m_windowCaption(windowCaption),
    m_unit(unit),
    m_scale(scale),
    m_numberSet(numberSet),
    m_min(min_),
    m_max(max_),
    m_value(value),
    m_hardMin(hardMin),
    m_hardMax(hardMax),
    m_parametersFilter(parametersFilter)
{

}

QString OperatorParameterSlider::windowCaption() const
{
    return m_windowCaption;
}

Slider::Unit OperatorParameterSlider::unit() const
{
    return m_unit;
}

Slider::Scale OperatorParameterSlider::scale() const
{
    return m_scale;
}

Slider::NumberSet OperatorParameterSlider::numberSet() const
{
    return m_numberSet;
}

qreal OperatorParameterSlider::getMin() const
{
    return m_min;
}

qreal OperatorParameterSlider::getMax() const
{
    return m_max;
}

qreal OperatorParameterSlider::value() const
{
    return m_value;
}

qreal OperatorParameterSlider::hardMin() const
{
    return m_hardMin;
}

qreal OperatorParameterSlider::hardMax() const
{
    return m_hardMax;
}

uint OperatorParameterSlider::parametersFilter() const
{
    return m_parametersFilter;
}

QString OperatorParameterSlider::currentValue() const
{
    return QString("%0%1").arg(m_value).arg(Slider::unitToLocalizedString(m_unit));
}

void OperatorParameterSlider::setUnit(const Slider::Unit &unit)
{
    m_unit = unit;
}

void OperatorParameterSlider::setScale(const Slider::Scale &scale)
{
    m_scale = scale;
}

void OperatorParameterSlider::setMin(const qreal &min)
{
    m_min = min;
}

void OperatorParameterSlider::setMax(const qreal &max)
{
    m_max = max;
}

void OperatorParameterSlider::setValue(const qreal &value)
{
    m_value = value;
    dflDebug("Slider: emit setOutOfDate()");
    emit setOutOfDate();
}

QJsonObject OperatorParameterSlider::save()
{
    QJsonObject obj;
    obj["type"] = QString("slider");
    obj["name"] = m_name;
    obj["unit"] = QString(Slider::unitToString(m_unit));
    obj["scale"] = Slider::scaleToString(m_scale);
    obj["min"] = m_min;
    obj["max"] = m_max;
    obj["value"] = m_value;
    return obj;
}

void OperatorParameterSlider::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "slider" ) {
        dflWarning(tr("Slider: invalid parameter type"));
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        dflWarning(tr("Slider: invalid parameter name"));
        return;
    }
    m_unit = Slider::unitFromString(obj["unit"].toString());
    m_scale = Slider::scaleFromString(obj["scale"].toString());
    m_min = obj["min"].toDouble();
    m_max = obj["max"].toDouble();
    m_value = obj["value"].toDouble();

    emit updated();
}

