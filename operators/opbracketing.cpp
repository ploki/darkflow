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
#include "opbracketing.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "photo.h"
#include "ports.h"
#include <Magick++.h>

class WorkerBracketing : public OperatorWorker {
public:
    WorkerBracketing(qreal compensation, qreal high, qreal low,
                     QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_compensation(compensation),
        m_high(high),
        m_low(low)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        newPhoto.setTag(TAG_HDR_COMP,QString::number(m_compensation));
        newPhoto.setTag(TAG_HDR_HIGH,QString::number(m_high));
        newPhoto.setTag(TAG_HDR_LOW,QString::number(m_low));
        return newPhoto;
    }

private:
    qreal m_compensation, m_high, m_low;
};

OpBracketing::OpBracketing(Process *parent) :
    Operator(OP_SECTION_BLEND, QT_TRANSLATE_NOOP("Operator", "Bracketing"), Operator::All, parent),
    m_compensation(new OperatorParameterSlider("compensation", tr("Compensation"), tr("Bracketing Exposure Compensation"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<16, 1, 1, 1<<16, Slider::FilterExposureFromOne, this)),
    m_high(new OperatorParameterSlider("high", tr("Limit High"), tr("Bracketing Limit High"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, M_SQRT1_2l, 1./(1<<16), 1<<16, Slider::FilterExposureFromOne, this)),
    m_low(new OperatorParameterSlider("low", tr("Limit Low"), tr("Bracketing Limit Low"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1./(1<<8), 1./(1<<16), 1<<16, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_compensation);
    addParameter(m_high);
    addParameter(m_low);
}

OpBracketing *OpBracketing::newInstance()
{
    return new OpBracketing(m_process);
}

OperatorWorker *OpBracketing::newWorker()
{
    return new WorkerBracketing(m_compensation->value(),
                                m_high->value(),
                                m_low->value(),
                                m_thread, this);
}
