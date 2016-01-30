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
#include "opunsharpmask.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerUnsharpMask : public OperatorWorker {
public:
    WorkerUnsharpMask(qreal radius,
                      qreal sigma,
                      qreal amount,
                      qreal threshold,
                      QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_radius(radius),
        m_sigma(sigma),
        m_amount(amount),
        m_threshold(threshold)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().unsharpmask(m_radius, m_sigma, m_amount, m_threshold);
        return newPhoto;
    }

private:
    qreal m_radius;
    qreal m_sigma;
    qreal m_amount;
    qreal m_threshold;
};

OpUnsharpMask::OpUnsharpMask(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Unsharp Mask"), Operator::NonHDR, parent),
    m_radius(new OperatorParameterSlider("radius", tr("Radius"), tr("Unsharp Mask Radius"), Slider::Value, Slider::Logarithmic, Slider::Real, .1, 100, 1, .1, 1000, Slider::FilterPixels, this)),
    m_sigma(new OperatorParameterSlider("sigma", tr("Sigma"), tr("Unsharp Mask Sigma"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 1, 0, 1, Slider::FilterPercent, this)),
    m_amount(new OperatorParameterSlider("amount", tr("Amount"), tr("Unsharp Mask Amount"), Slider::Percent, Slider::Logarithmic, Slider::Real, 0.1, 10, 1, 0.01, 100, Slider::FilterPercent, this)),
    m_threshold(new OperatorParameterSlider("threshold", tr("Threshold"), tr("Unsharp Mask Threshold"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange, 1, 1./QuantumRange, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_radius);
    addParameter(m_sigma);
    addParameter(m_amount);
    addParameter(m_threshold);

}

OpUnsharpMask *OpUnsharpMask::newInstance()
{
    return new OpUnsharpMask(m_process);
}

OperatorWorker *OpUnsharpMask::newWorker()
{
    return new WorkerUnsharpMask(m_radius->value(),
                                 m_radius->value()*m_sigma->value(),
                                 m_amount->value(),
                                 m_threshold->value(),
                                 m_thread, this);
}
