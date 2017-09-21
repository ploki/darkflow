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
#include "opselectivelabfilter.h"
#include "operatorparameterselectivelab.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "selectivelabfilter.h"

class WorkerSelectiveLabFilter : public OperatorWorker {
public:
    WorkerSelectiveLabFilter(int hue, int coverage, qreal saturation, bool strict,
                             qreal exposure, bool insideSelection, QThread *thread, Operator *op)
        : OperatorWorker(thread, op),
          m_filter(new SelectiveLabFilter(hue, coverage, saturation, strict, exposure, insideSelection, false))
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_filter->applyOn(newPhoto);
        return newPhoto;
    }

private:
    SelectiveLabFilter *m_filter;
};

OpSelectiveLabFilter::OpSelectiveLabFilter(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Selective Lab Filter"), Operator::All, parent),
    m_selectiveLab(new OperatorParameterSelectiveLab("labSelection", tr("Selection"), tr("Selective Lab Filter"), 0, 0, false, 35, true, true, true, this)),
    m_saturation(new OperatorParameterSlider("saturation", tr("Saturation"), tr("Selective Lab Filter Saturation"), Slider::Percent, Slider::Linear, Slider::Real, 0, 2, 1, 0, 10, Slider::FilterNothing, this)),
    m_exposure(new OperatorParameterSlider("exposure", tr("Exposure"), tr("Selective Lab Filter Exposure"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<8), 1<<8, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this)),
    m_exposureSelection(new OperatorParameterDropDown("exposureSelection", tr("Exposure zone"), this, SLOT(exposureSelection(int)))),
    m_exposureSelectionValue(Inside)
{
    m_exposureSelection->addOption(DF_TR_AND_C("Inside"), Inside, true);
    m_exposureSelection->addOption(DF_TR_AND_C("Outside"), Outside);

    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_selectiveLab);
    addParameter(m_saturation);
    addParameter(m_exposure);
    addParameter(m_exposureSelection);
}

OpSelectiveLabFilter *OpSelectiveLabFilter::newInstance()
{
    return new OpSelectiveLabFilter(m_process);
}

OperatorWorker *OpSelectiveLabFilter::newWorker()
{
    return new WorkerSelectiveLabFilter(m_selectiveLab->hue(),
                                        m_selectiveLab->coverage(),
                                        m_saturation->value(),
                                        m_selectiveLab->strict(),
                                        m_exposure->value(),
                                        m_exposureSelectionValue==Inside, m_thread, this);
}

Algorithm *OpSelectiveLabFilter::getAlgorithm() const
{
    return new SelectiveLabFilter(m_selectiveLab->hue(),
                                  m_selectiveLab->coverage(),
                                  m_saturation->value(),
                                  m_selectiveLab->strict(),
                                  m_exposure->value(),
                                  m_exposureSelectionValue==Inside,
                                  false);
}

void OpSelectiveLabFilter::releaseAlgorithm(Algorithm *algo) const
{
    delete algo;
}

void OpSelectiveLabFilter::exposureSelection(int v)
{
    if ( m_exposureSelectionValue != v ) {
        m_exposureSelectionValue = Selection(v);
        setOutOfDate();
    }
}
