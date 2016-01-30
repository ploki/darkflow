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
#include "operatorworker.h"
#include "oppassthrough.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"

#include "operatorparameterslider.h"

class PassThrough : public OperatorWorker {
public:
    PassThrough(QThread *thread, Operator *op) :
    OperatorWorker(thread, op) {}

    Photo process(const Photo &photo, int, int) {
        return Photo(photo);
    }
};



OpPassThrough::OpPassThrough(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, QT_TRANSLATE_NOOP("Operator", "Pass Through"), Operator::All, parent),
    m_slider(new OperatorParameterSlider("scale", tr("scale"), tr("scale"),
                                         Slider::ExposureValue, Slider::Logarithmic,
                                         Slider::Real,
                                         1., 1<<4,
                                         1.,
                                         1./65535, 65535,
                                         Slider::FilterAll,this))
{
    addInput(new OperatorInput(tr("Images set 1"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Images set 2"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Images set 3"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Merge"), this));

    addParameter(m_slider);
}

OpPassThrough *OpPassThrough::newInstance()
{
    return new OpPassThrough(m_process);
}

OperatorWorker *OpPassThrough::newWorker()
{
    return new PassThrough(m_thread, this);
}
