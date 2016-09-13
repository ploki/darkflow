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
#include "opdftbackward.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "discretefouriertransform.h"

#include <Magick++.h>

class WorkerDFTBackward : public OperatorWorker {
    //bool m_outputHDR;
public:
    WorkerDFTBackward(bool outputHDR, QThread *thread, OpDFTBackward *op)
        : OperatorWorker(thread, op) /*,
          m_outputHDR(outputHDR) */
    {
        Q_UNUSED(outputHDR);
    }
    Photo process(const Photo &, int, int) { throw 0; }
    bool play_onInput(int idx) {
        Q_UNUSED(idx);
        Q_ASSERT( m_inputs.count() == 2 );
        if (m_inputs[0].count() == 0 || m_inputs[1].count() == 0 ) {
            emitFailure();
            return false;
        }
        int count = qMax(m_inputs[0].count(), m_inputs[1].count());
        for (int i = 0 ; i < count ; ++i ) {
            if (aborted()) {
                emitFailure();
                return false;
            }
            Photo& pMagnitude = m_inputs[0][i%m_inputs[0].count()];
            Photo& pPhase = m_inputs[1][i%m_inputs[1].count()];
            double normalization = pMagnitude.getTag(TAG_DFT_NORMALIZATION).toDouble();
            int o_w = pMagnitude.getTag(TAG_DFT_WIDTH).toInt();
            int o_h = pMagnitude.getTag(TAG_DFT_HEIGHT).toInt();
            int o_m = qMax(o_w, o_h);
            if (0 == normalization) {
                dflWarning(tr("Invalid DFT Normalization value"));
                normalization = 1;
            }
            DiscreteFourierTransform dft(pMagnitude.image(), pPhase.image(), pMagnitude.getScale(),normalization);
            Magick::Image iRes = dft.reverse(1);
            iRes = DiscreteFourierTransform::roll(iRes, -o_m/2, -o_m/2);
            iRes.crop(Magick::Geometry(o_w, o_h, (o_m-o_w)/2, (o_m-o_h)/2));
            Photo pRes(iRes, Photo::Linear);
            pRes.setIdentity(m_operator->uuid()+"-"+QString::number(i));
            pRes.setTag(TAG_NAME, pMagnitude.getTag(TAG_NAME));
            outputPush(0, pRes);
        }
        emitSuccess();
        return true;
    }
};


OpDFTBackward::OpDFTBackward(Process *parent)
    : Operator(OP_SECTION_FREQUENCY_DOMAIN, QT_TRANSLATE_NOOP("Operator", "Backward DFT"), Operator::All, parent),
      m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
      m_outputHDRValue(true)
{
    addInput(new OperatorInput(tr("Magnitude"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Phase"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Image"), this));
    m_outputHDR->addOption(DF_TR_AND_C("No"), false);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true, true);

    addParameter(m_outputHDR);

}

OpDFTBackward *OpDFTBackward::newInstance()
{
    return new OpDFTBackward(m_process);
}

OperatorWorker *OpDFTBackward::newWorker()
{
    return new WorkerDFTBackward(m_outputHDR, m_thread, this);
}

void OpDFTBackward::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}
