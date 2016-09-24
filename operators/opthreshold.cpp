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
#include "opthreshold.h"
#include "threshold.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "channelmixer.h"
#include "cielab.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerThreshold : public OperatorWorker {
public:
    WorkerThreshold(qreal high, qreal low, OpThreshold::Component component,
                    QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_threshold(high, low),
        m_channelMixer(LUMINANCE_RED, LUMINANCE_GREEN, LUMINANCE_BLUE),
        m_component(component)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        if (m_component == OpThreshold::ComponentLuminosity)
            m_channelMixer.applyOn(newPhoto);
        m_threshold.applyOn(newPhoto);
        return newPhoto;
    }

private:
    Threshold m_threshold;
    ChannelMixer m_channelMixer;
    OpThreshold::Component m_component;
};

OpThreshold::OpThreshold(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Threshold"), Operator::NonHDR, parent),
    m_high(new OperatorParameterSlider("high", tr("High"), tr("Threshold High"),Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange, 1, 1, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this)),
    m_low(new OperatorParameterSlider("low", tr("Low"), tr("Threshold Low"),Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange, 1, 1, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this)),
    m_component(new OperatorParameterDropDown("component", tr("Component"), this, SLOT(selectComponent(int)))),
    m_componentValue(ComponentLuminosity)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_component->addOption(DF_TR_AND_C("Luminosity"), ComponentLuminosity, true);
    m_component->addOption(DF_TR_AND_C("RGB"), ComponentRGB);

    addParameter(m_high);
    addParameter(m_low);
    addParameter(m_component);
}

OpThreshold *OpThreshold::newInstance()
{
    return new OpThreshold(m_process);
}

OperatorWorker *OpThreshold::newWorker()
{
    return new WorkerThreshold(m_high->value(),
                               m_low->value(),
                               m_componentValue,
                               m_thread, this);
}

void OpThreshold::selectComponent(int v)
{
    if ( m_componentValue != Component(v) ) {
        m_componentValue = Component(v);
        setOutOfDate();
    }
}
