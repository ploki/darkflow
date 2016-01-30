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
#include "opmodulate.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>
#include "photo.h"

using Magick::Quantum;

class WorkerModulate : public OperatorWorker {
public:
    WorkerModulate(qreal hue, qreal saturation, qreal value, QThread *thread, OpModulate *op) :
        OperatorWorker(thread, op),
        m_hue(hue),
        m_saturation(saturation),
        m_value(value)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        Magick::Image& image = newPhoto.image();
        image.modulate(m_value, m_saturation, m_hue);
        newPhoto.curve().modulate(m_value, 100, 100);
        return newPhoto;
    }
private:
    qreal m_hue;
    qreal m_saturation;
    qreal m_value;
};


OpModulate::OpModulate(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Modulate"), Operator::NonHDR, parent),
    m_hue(new OperatorParameterSlider("hue", tr("Hue"), tr("Modulate Hue"),Slider::Value, Slider::Linear, Slider::Integer, -180, 180, 0, -180, 180, Slider::FilterNothing, this)),
    m_saturation(new OperatorParameterSlider("saturation", tr("Saturation"), tr("Modulate Saturation"),Slider::Percent, Slider::Linear, Slider::Integer, 0, 2, 1, 0, 10, Slider::FilterNothing, this)),
    m_value(new OperatorParameterSlider("value", tr("Exposure"), tr("Modulate Exposure"),Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<8, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    addParameter(m_hue);
    addParameter(m_saturation);
    addParameter(m_value);
}

OpModulate *OpModulate::newInstance()
{
    return new OpModulate(m_process);
}

OperatorWorker *OpModulate::newWorker()
{
    return new WorkerModulate(100.+m_hue->value()/1.8,
                              m_saturation->value()*100.,
                              m_value->value()*100., m_thread, this);
}
