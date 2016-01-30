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
#include "ophotpixels.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "hotpixels.h"
#include "ports.h"

class WorkerHotPixels : public OperatorWorker {
public:
    WorkerHotPixels(qreal delta, bool aggressive, bool naive, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_hotPixels(delta,aggressive, naive)
    {
    }
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_hotPixels.applyOn(newPhoto);
        return newPhoto;
    }

private:
    HotPixels m_hotPixels;
};

OpHotPixels::OpHotPixels(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Hot Pixels"), Operator::NonHDR, parent),
    m_delta(new OperatorParameterSlider("delta", tr("Delta"), tr("Hot Pixels Delta"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, M_SQRT2l, 1, 1<<16, Slider::FilterExposureFromOne, this)),
    m_aggressive(new OperatorParameterDropDown("aggressive", tr("Aggressive"), this, SLOT(selectAggressive(int)))),
    m_naive(new OperatorParameterDropDown("naive", tr("Naive"), this, SLOT(selectNaive(int)))),
    m_aggressiveValue(true),
    m_naiveValue(false)
{
    m_aggressive->addOption(DF_TR_AND_C("Yes"), true, true);
    m_aggressive->addOption(DF_TR_AND_C("No"), false);
    m_naive->addOption(DF_TR_AND_C("Yes"), true);
    m_naive->addOption(DF_TR_AND_C("No"), false, true);

    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    addParameter(m_delta);
    addParameter(m_aggressive);
    addParameter(m_naive);
}

OpHotPixels *OpHotPixels::newInstance()
{
    return new OpHotPixels(m_process);
}

OperatorWorker *OpHotPixels::newWorker()
{
return new WorkerHotPixels(m_delta->value(),
                           m_aggressiveValue,
                           m_naiveValue,
                           m_thread, this);
}

void OpHotPixels::selectAggressive(int v)
{
    if ( m_aggressiveValue != !!v) {
        m_aggressiveValue = !!v;
        setOutOfDate();
    }
}

void OpHotPixels::selectNaive(int v)
{
    if ( m_naiveValue != !!v) {
        m_naiveValue = !!v;
        setOutOfDate();
    }
}
