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
#ifndef OPERATORPARAMETERSLIDER_H
#define OPERATORPARAMETERSLIDER_H

#include "operatorparameter.h"
#include "slider.h"

class OperatorParameterSlider : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterSlider(
            const QString &name,
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
            Operator *op);

    QString windowCaption() const;
    Slider::Unit unit() const;
    Slider::Scale scale() const;
    Slider::NumberSet numberSet() const;
    qreal getMin() const;
    qreal getMax() const;
    qreal value() const;
    qreal hardMin() const;
    qreal hardMax() const;
    uint parametersFilter() const;
    QString currentValue() const;

    QJsonObject save(const QString& baseDirStr);
    void load(const QJsonObject &obj);

public slots:
    void setUnit(const Slider::Unit &unit);
    void setScale(const Slider::Scale &scale);
    void setMin(const qreal &min);
    void setMax(const qreal &max);
    void setValue(const qreal &value);

signals:
    void updated();

private:
    QString m_windowCaption;
    Slider::Unit m_unit;
    Slider::Scale m_scale;
    Slider::NumberSet m_numberSet;
    qreal m_min;
    qreal m_max;
    qreal m_value;
    qreal m_hardMin;
    qreal m_hardMax;
    uint m_parametersFilter;
};

#endif // OPERATORPARAMETERSLIDER_H
