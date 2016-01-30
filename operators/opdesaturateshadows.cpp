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
#include "opdesaturateshadows.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "desaturateshadows.h"
#include "photo.h"
#include <Magick++.h>

using Magick::Quantum;
class WorkerDeSha : public OperatorWorker {
public:
    WorkerDeSha(qreal highlightLimit, qreal range, qreal saturation, QThread *thread, OpDesaturateShadows *op) :
        OperatorWorker(thread, op),
        m_desha(highlightLimit,range, saturation)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_desha.applyOn(newPhoto);

        //nonsens
        //m_desha.applyOnImage(newPhoto.curve());
        return newPhoto;
    }

private:
    DesaturateShadows m_desha;
};

OpDesaturateShadows::OpDesaturateShadows(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Desaturate Shadows"), Operator::All, parent),
    m_highlightLimit(new OperatorParameterSlider("highlightLimit", tr("Higher limit"), tr("Desaturate Shadows Higher Limit"),Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<8),1, 1./(1<<5),1./QuantumRange,1, Slider::FilterExposureFromOne, this)),
    m_range(new OperatorParameterSlider("range", tr("Range"), tr("Desaturate Shadows Range"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<8, 1<<3, 1, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_saturation(new OperatorParameterSlider("saturation", tr("Saturation"), tr("Desaturate Shadows Saturation"), Slider::Percent, Slider::Linear, Slider::Integer, 0, 1, 0, 0, 1, Slider::FilterNothing, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    addParameter(m_highlightLimit);
    addParameter(m_range);
    addParameter(m_saturation);
}

OpDesaturateShadows *OpDesaturateShadows::newInstance()
{
    return new OpDesaturateShadows(m_process);
}

OperatorWorker *OpDesaturateShadows::newWorker()
{
    return new WorkerDeSha(m_highlightLimit->value(),
                           m_range->value(),
                           m_saturation->value(),
                           m_thread, this);
}
