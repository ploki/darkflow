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
#include "opcolorfilter.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "colorfilter.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerColorFilter : public OperatorWorker {
public:
    WorkerColorFilter(qreal r, qreal g, qreal b, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_colorFilter(r,g,b)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_colorFilter.applyOn(newPhoto);
        return newPhoto;
    }

private:
    ColorFilter m_colorFilter;
};

OpColorFilter::OpColorFilter(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Color Filter"), Operator::All, parent),
    m_r(new OperatorParameterSlider("red", tr("Red"), tr("Color Filter Red Component"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_g(new OperatorParameterSlider("green", tr("Green"), tr("Color Filter Green Component"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_b(new OperatorParameterSlider("blue", tr("Blue"), tr("Color Filter Blue Component"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_r);
    addParameter(m_g);
    addParameter(m_b);
}

OpColorFilter *OpColorFilter::newInstance()
{
    return new OpColorFilter(m_process);
}

OperatorWorker *OpColorFilter::newWorker()
{
    return new WorkerColorFilter(m_r->value(), m_g->value(), m_b->value(), m_thread, this);
}
