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
#include "opblackbody.h"
#include "operatorworker.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "whitebalance.h"
#include "photo.h"
#include "hdr.h"
#include  <Magick++.h>

using Magick::Quantum;

class WorkerBlackBody : public OperatorWorker {
public:
    WorkerBlackBody(qreal temperature, qreal tint, qreal value, bool outputHDR,
                    QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_temperature(temperature),
        m_tint(tint),
        m_value(value),
        m_outputHDR(outputHDR)
    {}
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play() {
        Photo photo(Photo::Linear);
        photo.setIdentity(m_operator->uuid());
        photo.createImage(1,1);
        if ( photo.isComplete() ) {
            double rgb[3];
            WhiteBalance::Temperature_to_RGB(m_temperature, rgb);
            rgb[1]/=m_tint;
            double div=rgb[0];
            if (div < rgb[1] ) div=rgb[1];
            if (div < rgb[2] ) div=rgb[2];
            rgb[0]/=div;
            rgb[1]/=div;
            rgb[2]/=div;
            rgb[0]*=m_value;
            rgb[1]*=m_value;
            rgb[2]*=m_value;
            if (m_outputHDR) {
                photo.image().pixelColor(0, 0,
                         Magick::Color(
                            toHDR(rgb[0]*QuantumRange),
                            toHDR(rgb[1]*QuantumRange),
                            toHDR(rgb[2]*QuantumRange)));
                photo.setScale(Photo::HDR);
            } else {
                photo.image().pixelColor(0, 0,
                         Magick::Color(
                            clamp<quantum_t>(rgb[0]*QuantumRange, 0, QuantumRange),
                            clamp<quantum_t>(rgb[1]*QuantumRange, 0, QuantumRange),
                            clamp<quantum_t>(rgb[2]*QuantumRange, 0, QuantumRange)));
            }
            //dflDebug(QString("r: %0, g: %1, b: %2").arg(rgb[0]).arg(rgb[1]).arg(rgb[2]).toLatin1());
            photo.setTag(TAG_NAME, QString("Black Body %0 K").arg(m_temperature));
            outputPush(0, photo);
            emitSuccess();
        }
        else
            emitFailure();
    }

private:
    qreal m_temperature;
    qreal m_tint;
    qreal m_value;
    bool m_outputHDR;
};

OpBlackBody::OpBlackBody(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Black Body"), Operator::NA, parent),
    m_temperature(new OperatorParameterSlider("temperature", tr("Temperature"), tr("Black Body Temperature"),
                                              Slider::Value, Slider::Logarithmic, Slider::Integer,
                                              2000, 12000, 6500, 2000, 12000, Slider::FilterNothing,this)),
    m_tint(new OperatorParameterSlider("tint", tr("Green tint"), tr("Black Body Green Tint"),
                                       Slider::Percent, Slider::Logarithmic, Slider::Real,
                                       0.5, 2, 1, 0.01, 100, Slider::FilterNothing, this)),
    m_value(new OperatorParameterSlider("intensity", tr("Intensity"), tr("Black Body Value"),
                                        Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                        1./(1<<8), 1, .125, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this)),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    addOutput(new OperatorOutput(tr("Black body"), this));

    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);

    addParameter(m_temperature);
    addParameter(m_tint);
    addParameter(m_value);
    addParameter(m_outputHDR);

}

OpBlackBody *OpBlackBody::newInstance()
{
    return new OpBlackBody(m_process);
}

OperatorWorker *OpBlackBody::newWorker()
{
    return new WorkerBlackBody(m_temperature->value(),
                               m_tint->value(),
                               m_value->value(),
                               m_outputHDRValue,
                               m_thread, this);
}

void OpBlackBody::setOutputHDR(int v)
{
    if (m_outputHDRValue != !!v) {
        m_outputHDRValue = !!v;
        setOutOfDate();
    }
}
