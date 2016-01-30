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
#include "opchannelmixer.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "channelmixer.h"
#include "cielab.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerChannelMixer : public OperatorWorker {
public:
    WorkerChannelMixer(qreal r, qreal g, qreal b, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_channelMixer(r,g,b)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_channelMixer.applyOn(newPhoto);
        return newPhoto;
    }

private:
    ChannelMixer m_channelMixer;
};

OpChannelMixer::OpChannelMixer(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Channel Mixer"), Operator::All, parent),
    m_r(new OperatorParameterSlider("red", tr("Red"), tr("Channel Mixer Red Component"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, LUMINANCE_RED, 0, 1, Slider::FilterPercent, this)),
    m_g(new OperatorParameterSlider("green", tr("Green"), tr("Channel Mixer Green Component"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, LUMINANCE_GREEN, 0, 1, Slider::FilterPercent, this)),
    m_b(new OperatorParameterSlider("blue", tr("Blue"), tr("Channel Mixer Blue Component"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, LUMINANCE_BLUE, 0, 1, Slider::FilterPercent, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_r);
    addParameter(m_g);
    addParameter(m_b);
}

OpChannelMixer *OpChannelMixer::newInstance()
{
    return new OpChannelMixer(m_process);
}

OperatorWorker *OpChannelMixer::newWorker()
{
    return new WorkerChannelMixer(m_r->value(), m_g->value(), m_b->value(), m_thread, this);
}
