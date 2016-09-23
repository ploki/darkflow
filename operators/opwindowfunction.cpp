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

#include "opwindowfunction.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "operatorparameterdropdown.h"
#include "discretefouriertransform.h"

class WindowFunctionWorker : public OperatorWorker {
    DiscreteFourierTransform::WindowFunction m_function;
public:
    WindowFunctionWorker(DiscreteFourierTransform::WindowFunction function,
                         QThread *thread,
                         Operator *op) :
        OperatorWorker(thread, op),
        m_function(function) {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        Magick::Image& image = newPhoto.image();
        image = DiscreteFourierTransform::window(image, m_function);
        return newPhoto;
    }
};

OpWindowFunction::OpWindowFunction(Process *parent) :
    Operator(OP_SECTION_FREQUENCY_DOMAIN, QT_TRANSLATE_NOOP("Operator", "Window Function"), Operator::All, parent),
    m_function(new OperatorParameterDropDown("function", tr("Function"), this, SLOT(selectFunction(int)))),
    m_functionValue(DiscreteFourierTransform::WindowHamming)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_function->addOption(DF_TR_AND_C("None"), DiscreteFourierTransform::WindowNone, false);
    m_function->addOption(DF_TR_AND_C("Hamming"), DiscreteFourierTransform::WindowHamming, false);
    m_function->addOption(DF_TR_AND_C("Hann"), DiscreteFourierTransform::WindowHann, false);
    m_function->addOption(DF_TR_AND_C("Nuttal"), DiscreteFourierTransform::WindowNuttal, false);
    m_function->addOption(DF_TR_AND_C("Blackman-Nuttal"), DiscreteFourierTransform::WindowBlackmanNuttal, false);
    m_function->addOption(DF_TR_AND_C("Blackman-Harris"), DiscreteFourierTransform::WindowBlackmanHarris, false);
    addParameter(m_function);
}

OpWindowFunction *OpWindowFunction::newInstance()
{
    return new OpWindowFunction(m_process);
}

OperatorWorker *OpWindowFunction::newWorker()
{
    return new WindowFunctionWorker(DiscreteFourierTransform::WindowFunction(m_functionValue),
                                    m_thread, this);
}

void OpWindowFunction::selectFunction(int v)
{
    if (m_functionValue != v) {
        m_functionValue = v;
        setOutOfDate();
    }
}

