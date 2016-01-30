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
#include "oplevel.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"

using Magick::Quantum;

class WorkerLevel : public OperatorWorker {
public:
    WorkerLevel(qreal blackPoint, qreal whitePoint, qreal gamma, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_blackPoint(blackPoint),
        m_whitePoint(whitePoint),
        m_gamma(gamma)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().level(m_blackPoint*QuantumRange,
                               m_whitePoint*QuantumRange,
                               m_gamma);
        newPhoto.curve().level(m_blackPoint*QuantumRange,
                               m_whitePoint*QuantumRange,
                               m_gamma);
        return newPhoto;
    }

private:
    qreal m_blackPoint;
    qreal m_whitePoint;
    qreal m_gamma;
};

OpLevel::OpLevel(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "Level"), Operator::NonHDR, parent),
    m_blackPoint(new OperatorParameterSlider("blackPoint", tr("Black Point"), tr("Level Black Point"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1./(1<<16), 1./(1<<16), 1, Slider::FilterExposureFromOne, this)),
    m_whitePoint(new OperatorParameterSlider("blackPoint", tr("White Point"), tr("Level White Point"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1, 1./(1<<16), 1, Slider::FilterExposureFromOne, this)),
    m_gamma(new OperatorParameterSlider("gamma", tr("Gamma"), tr("Level Gamma"), Slider::Value, Slider::Logarithmic, Slider::Real, 0.1, 10, 1, 0.01, 10, Slider::FilterNothing, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_blackPoint);
    addParameter(m_whitePoint);
    addParameter(m_gamma);
}

OpLevel *OpLevel::newInstance()
{
    return new OpLevel(m_process);
}

OperatorWorker *OpLevel::newWorker()
{
return new WorkerLevel(m_blackPoint->value(),
                       m_whitePoint->value(),
                       m_gamma->value(),
                       m_thread, this);
}
