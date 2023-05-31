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
#include "opdftforward.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "discretefouriertransform.h"
#include <QtMath>
#include <Magick++.h>

class WorkerDFTForward : public OperatorWorker {
    bool m_outputHDR;
public:
    WorkerDFTForward(bool outputHDR, QThread *thread, OpDFTForward *op)
        : OperatorWorker(thread, op),
          m_outputHDR(outputHDR)
    {}
    Photo process(const Photo& photo, int, int) {
        Magick::Image source(photo.image());
        int w = source.columns(),
            h = source.rows(),
            m = qNextPowerOfTwo(qMax(w, h));
        source = DiscreteFourierTransform::normalize(source, m, true);
        source = DiscreteFourierTransform::roll(source, m/2, m/2);
        DiscreteFourierTransform dft(source, photo.getScale());
        Photo::Gamma scale = m_outputHDR ? Photo::HDR : Photo::Linear;
        double normalization;
        Photo magnitude(dft.imageMagnitude(scale, &normalization), scale);
        magnitude.setScale(scale);
        magnitude.setTag(TAG_DFT_NORMALIZATION, QString::number(normalization));
        magnitude.setTag(TAG_DFT_WIDTH, QString::number(w));
        magnitude.setTag(TAG_DFT_HEIGHT, QString::number(h));
        magnitude.setIdentity(photo.getIdentity()+"-m");
        magnitude.setTag(TAG_NAME, tr("Magnitude"));
        Photo phase(dft.imagePhase(), scale);
        phase.setIdentity(photo.getIdentity()+"-p");
        phase.setTag(TAG_NAME, tr("Phase"));
        outputPush(1, phase);
        return magnitude;
    }
};

OpDFTForward::OpDFTForward(Process *parent)
    : Operator(OP_SECTION_FREQUENCY_DOMAIN, QT_TRANSLATE_NOOP("Operator", "Forward DFT"), Operator::All, parent),
      m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
      m_outputHDRValue(true)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Magnitude"), this));
    addOutput(new OperatorOutput(tr("Phase"), this));

    m_outputHDR->addOption(DF_TR_AND_C("No"), false);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true, true);

    addParameter(m_outputHDR);
}

OpDFTForward *OpDFTForward::newInstance()
{
    return new OpDFTForward(m_process);
}

OperatorWorker *OpDFTForward::newWorker()
{
    return new WorkerDFTForward(m_outputHDRValue, m_thread, this);
}

void OpDFTForward::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}
