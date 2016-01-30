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
#include "opadaptivethreshold.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>
#include "hdr.h"
#include "igamma.h"

using Magick::Quantum;

class WorkerAdaptiveThreshold : public OperatorWorker {
public:
    WorkerAdaptiveThreshold(int width, int height, quantum_t offset, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_width(width),
        m_height(height),
        m_offset(offset)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().adaptiveThreshold(m_width, m_height, m_offset);
        return newPhoto;
    }
private:
    int m_width;
    int m_height;
    quantum_t m_offset;
};

OpAdaptiveThreshold::OpAdaptiveThreshold(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Adaptive Threshold"), Operator::NonHDR, parent),
    m_width(new OperatorParameterSlider("width", tr("Width"), tr("Adaptive Threshold Width"), Slider::Value, Slider::Linear, Slider::Integer, 1, 25, 4, 1, 1000, Slider::FilterNothing, this)),
    m_height(new OperatorParameterSlider("height", tr("Height"), tr("Adaptive Threshold Height"), Slider::Value, Slider::Linear, Slider::Integer, 1, 25, 4, 1, 1000, Slider::FilterNothing, this)),
    m_offset(new OperatorParameterSlider("offset", tr("Offset"), tr("Adaptive Threshold Offset"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1, 1<<16, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_width);
    addParameter(m_height);
    addParameter(m_offset);
}

OpAdaptiveThreshold *OpAdaptiveThreshold::newInstance()
{
    return new OpAdaptiveThreshold(m_process);
}

OperatorWorker *OpAdaptiveThreshold::newWorker()
{
    return new WorkerAdaptiveThreshold(DF_ROUND(m_width->value()),
                                       DF_ROUND(m_height->value()),
                                       (m_offset->value()-1.),
                                       m_thread, this);
}
