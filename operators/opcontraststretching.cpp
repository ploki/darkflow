/*
 * Copyright (c) 2006-2021, Guillaume Gimenez <guillaume@blackmilk.fr>
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
#include "opcontraststretching.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "contraststretching.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerContrastStretching : public OperatorWorker {
public:
    WorkerContrastStretching(qreal bp, qreal xl, qreal yl, qreal xh, qreal yh,
                             QThread *thread, OpContrastStretching *op) :
        OperatorWorker(thread, op),
        m_contrastStretching(bp, xl, yl, xh, yh)
    {

    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_contrastStretching.applyOn(newPhoto);
        return newPhoto;
    }
private:
    ContrastStretching m_contrastStretching;
};

/*
 * exposure
 * gamma
 * highlights
 * shadows
 * blackpoint
 */

OpContrastStretching::OpContrastStretching(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "Stretch contrast"), Operator::All, parent),
    m_exposure(new OperatorParameterSlider("exposure",     tr("Exposure"),    tr("Stretch contrast exposure"),     Slider::ExposureValue, Slider::Logarithmic, Slider::Real,               1, QuantumRange, 1.,              1., QuantumRange, Slider::FilterExposureFromOne, this)),
    m_gamma(new OperatorParameterSlider("gamma",           tr("Gamma"),       tr("Stretch contrast gamma"),        Slider::Value,         Slider::Logarithmic, Slider::Real,          1./16.,          16., 1., 1./QuantumRange, QuantumRange, Slider::FilterNothing, this)),
    m_highlights(new OperatorParameterSlider("highlights", tr("Highlights"),  tr("Stretch contrast highlights"),   Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange,           1., 1., 1./QuantumRange,            1, Slider::FilterExposureFromOne, this)),
    m_shadows(new OperatorParameterSlider("shadows",       tr("Shadows"),     tr("Stretch contrast shadows"),      Slider::ExposureValue, Slider::Logarithmic, Slider::Real,              1., QuantumRange, 1.,              1., QuantumRange, Slider::FilterExposureFromOne, this)),
    m_blackpoint(new OperatorParameterSlider("blackpoint", tr("Black point"), tr("Stretch contrast black point"),  Slider::ExposureValue, Slider::Logarithmic, Slider::Real,              1., QuantumRange, 1.,              1., QuantumRange, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_exposure);
    addParameter(m_gamma);
    addParameter(m_highlights);
    addParameter(m_shadows);
    addParameter(m_blackpoint);
}

OpContrastStretching *OpContrastStretching::newInstance()
{
    return new OpContrastStretching(m_process);
}

OperatorWorker *OpContrastStretching::newWorker()
{
    qreal exposure =   log(m_exposure->value())/log(QuantumRange + 1),   // 0 - 1
          gamma =      m_gamma->value(),                  //
          highlights = log(m_highlights->value())/log(QuantumRange + 1), // -1 - 0
          shadows =    log(m_shadows->value())/log(QuantumRange + 1),    // 0 - 1
          blackpoint = log(m_blackpoint->value())/log(QuantumRange + 1); // 0 - 1

    /*
    f(x) = a * x + b
    f(1-exp) = 1
    a * (1-exp) + b = 1
    b = 1 - a * (1-exp)
    */
    // f(x) = x/gamma + f0
    // f(1-exp)
    // f0 = 1 - (1 - exp)/gamma    //
    qreal slope = 1./gamma,
            yh = 1. + highlights,
            yl = shadows + blackpoint,
            f0 = 1. - slope * ( 1. - exposure),
            xh = (yh-f0)/slope,
            xl = (yl-f0)/slope;

    if (xl < 0) {
        xl = 0;
        yl = f0;
    }

    if (xh < 0) {
        xh = 0;
        yh = f0;
    }
    qDebug("*****");
    qDebug("slope=%f", slope);
    qDebug("(xl,yl)=(%f, %f)", (16. * xl), (16. * yl));
    qDebug("(xh,yh)=(%f, %f", (16. * xh), (16. * yh));
    qDebug("f0=%f",(16.* f0));
    return new WorkerContrastStretching(pow(QuantumRange + 1, blackpoint)/qreal(QuantumRange+1),
                                        pow(QuantumRange + 1, xl)/qreal(QuantumRange+1),
                                        pow(QuantumRange + 1, yl)/qreal(QuantumRange+1),
                                        pow(QuantumRange + 1, xh)/qreal(QuantumRange+1),
                                        pow(QuantumRange + 1, yh)/qreal(QuantumRange+1),
                                        m_thread, this);
}
