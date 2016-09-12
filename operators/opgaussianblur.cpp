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
#include "opgaussianblur.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerGaussianBlur : public OperatorWorker {
public:
    WorkerGaussianBlur(qreal radius,
                       qreal sigma,
                       QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_radius(radius),
        m_sigma(sigma)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().gaussianBlur(m_radius, m_sigma);
        return newPhoto;
    }

private:
    qreal m_radius;
    qreal m_sigma;
};

OpGaussianBlur::OpGaussianBlur(Process *parent) :
    Operator(OP_SECTION_FREQUENCY_DOMAIN, QT_TRANSLATE_NOOP("Operator", "Gaussian Blur"), Operator::NonHDR, parent),
    m_radius(new OperatorParameterSlider("radius", tr("Radius"), tr("Gaussian Blur Radius"), Slider::Value, Slider::Logarithmic, Slider::Real, .1, 100, 1, .1, 1000, Slider::FilterPixels, this)),
    m_sigma(new OperatorParameterSlider("sigma", tr("Sigma"), tr("Gaussian Blur Sigma"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 1, 0, 1, Slider::FilterPercent, this))

{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_radius);
    addParameter(m_sigma);

}

OpGaussianBlur *OpGaussianBlur::newInstance()
{
    return new OpGaussianBlur(m_process);
}

OperatorWorker *OpGaussianBlur::newWorker()
{
    return new WorkerGaussianBlur(m_radius->value(),
                                 m_radius->value()*m_sigma->value(),
                                 m_thread, this);
}
