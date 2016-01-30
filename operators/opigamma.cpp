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
#include "opigamma.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "igamma.h"
#include "preferences.h"

class WorkerIGamma : public OperatorWorker {
public:
    WorkerIGamma(qreal gamma, qreal x0, bool invert, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_iGamma(gamma, x0, invert),
        m_invert(invert)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        if ( m_invert && photo.getScale() != Photo::NonLinear )
            dflWarning(photo.getIdentity()+": must be Non-linear");
        if ( !m_invert && photo.getScale() == Photo::NonLinear)
            dflWarning(photo.getIdentity()+": must be Linear or HDR");

        m_iGamma.applyOn(newPhoto);
        return newPhoto;
    }
private:
    iGamma m_iGamma;
    bool m_invert;
};

OpIGamma::OpIGamma(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "iGamma"), Operator::All, parent),
    m_gamma(0),
    m_dynamicRange(0),
    m_revert(false),
    m_revertDialog(new OperatorParameterDropDown("revert", tr("Revert"), this, SLOT(revert(int))))
{
    qreal defaultGamma;
    qreal defaultDR;
    qreal defaultMax = 1<<12;
    switch (preferences->getCurrentTarget()) {
    case Preferences::Linear:
        defaultGamma = 1;
        defaultMax = defaultDR = 1<<16;
        break;
    case Preferences::sRGB:
        defaultGamma = 2.4;
        defaultDR = 1./0.00304L;
        break;
    case Preferences::IUT_BT_709:
        defaultGamma = 2.222L;
        defaultDR = 1./0.018L;
        break;
    case Preferences::SquareRoot:
        defaultGamma = 2;
        defaultMax = defaultDR = 1<<16;
        break;
    default:
        defaultGamma = 1;
        defaultMax = defaultDR = 1<<16;
        break;
    }

    m_gamma = new OperatorParameterSlider("gamma", tr("Gamma"), tr("Gamma Power"),
                                          Slider::Value, Slider::Logarithmic, Slider::Real,
                                          0.1, 10, defaultGamma, 0.01, 100, Slider::FilterNothing,this);
    m_dynamicRange = new OperatorParameterSlider("logarithmicRange", tr("Logarithmic on"), tr("Gamma Logarithmic Range"),
                                                 Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                                 1, defaultMax, defaultDR, 1, 1<<16, Slider::FilterExposure, this);

    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_revertDialog->addOption(DF_TR_AND_C("No"), false, true);
    m_revertDialog->addOption(DF_TR_AND_C("Yes"), true);

    addParameter(m_gamma);
    addParameter(m_dynamicRange);
    addParameter(m_revertDialog);

}

OpIGamma *OpIGamma::newInstance()
{
    return new OpIGamma(m_process);
}

OperatorWorker *OpIGamma::newWorker()
{
    return new WorkerIGamma(m_gamma->value(), 1./m_dynamicRange->value(), m_revert, m_thread, this);
}

void OpIGamma::revert(int v)
{
    if ( m_revert != !!v ) {
        m_revert = !!v;
        setOutOfDate();
    }
}
