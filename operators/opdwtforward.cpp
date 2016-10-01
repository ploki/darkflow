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
#include "opdwtforward.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "atrouswavelettransform.h"

class WorkerDWTForward : public OperatorWorker {
    OpDWTForward::Algorithm m_algorithm;
    OpDWTForward::Wavelet m_wavelet;
    int m_planes;
    bool m_outputHDR;
public:
    WorkerDWTForward(OpDWTForward::Algorithm algorithm,
                     OpDWTForward::Wavelet wavelet,
                     int planes, bool outputHDR,
                     QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_algorithm(algorithm),
        m_wavelet(wavelet),
        m_planes(planes),
        m_outputHDR(outputHDR)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        const double *wavelet = NULL;
        int order = 0;
        switch (m_wavelet) {
        case OpDWTForward::WaveletLinear:
            wavelet = linearWavelet;
            order = sizeof(linearWavelet)/sizeof(*linearWavelet);
            break;
        case OpDWTForward::WaveletB3Spline:
            wavelet = b3SplineWavelet;
            order = sizeof(b3SplineWavelet)/sizeof(*b3SplineWavelet);
            break;
        }
        if (m_algorithm != OpDWTForward::AlgorithmATrous)
            throw 0;
        for (int i = 0, s = m_inputs[0].count() ; i < s ; ++i ) {
            Photo photo(m_inputs[0][i]);
            Photo sign(photo);
            ATrousWaveletTransform dwt(photo, wavelet, order);
            for (int n = 0 ; n < m_planes ; ++n) {
                outputPush(n, dwt.transform(n, m_planes,
                                            m_outputHDR
                                            ? Photo::HDR
                                            : Photo::Linear,
                                            sign));
                outputPush(m_planes, sign);
                emitProgress(i, s, n, m_planes);
            }
        }
        emitSuccess();
    }
};


static const char *AlgorithmStr[] = {
    QT_TRANSLATE_NOOP("OpDWTForward", "À trous")
};

static const char *WaveletStr[] = {
    QT_TRANSLATE_NOOP("OpDWTForward", "Linear"),
    QT_TRANSLATE_NOOP("OpDWTForward", "B3-spline"),
};

OpDWTForward::OpDWTForward(int nPlanes, Process *parent) :
    Operator(OP_SECTION_FREQUENCY_DOMAIN,
                QT_TRANSLATE_NOOP("Operator", "%0-way Forward DWT"),
                Operator::All, parent),
    m_planes(nPlanes),
    m_algorithm(new OperatorParameterDropDown("algorithm", tr("Algorithm"), this, SLOT(selectAlgorithm(int)))),
    m_algorithmValue(AlgorithmATrous),
    m_wavelet(new OperatorParameterDropDown("wavelet", tr("Wavelet"), this, SLOT(selectWavelet(int)))),
    m_waveletValue(WaveletB3Spline),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(selectOutputHDR(int)))),
    m_outputHDRValue(true)
{
    m_classIdentifier = m_classIdentifier.arg(m_planes);
    m_name = m_name.arg(m_planes);
    m_localizedClassIdentifier= m_localizedClassIdentifier.arg(m_planes);

    addInput(new OperatorInput(tr("Image"), OperatorInput::Set, this));
    for (int i = 1 ; i <= m_planes ; ++i ) {
        QString name = tr("Plane %0").arg(i);
        addOutput(new OperatorOutput(name, this));
    }
    addOutput(new OperatorOutput(tr("Sign"), this));
    m_algorithm->addOption(DF_TR_AND_C(AlgorithmStr[AlgorithmATrous]), AlgorithmATrous, true);

    m_wavelet->addOption(DF_TR_AND_C(WaveletStr[WaveletLinear]), WaveletLinear);
    m_wavelet->addOption(DF_TR_AND_C(WaveletStr[WaveletB3Spline]), WaveletB3Spline, true);

    m_outputHDR->addOption(DF_TR_AND_C("No"), false);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true, true);

    addParameter(m_algorithm);
    addParameter(m_wavelet);
    addParameter(m_outputHDR);
}

OpDWTForward *OpDWTForward::newInstance()
{
    return new OpDWTForward(m_planes, m_process);
}

OperatorWorker *OpDWTForward::newWorker()
{
    return new WorkerDWTForward(m_algorithmValue, m_waveletValue, m_planes,
                                m_outputHDRValue, m_thread, this);
}

void OpDWTForward::selectAlgorithm(int v)
{
    if ( m_algorithmValue != Algorithm(v) ) {
        m_algorithmValue = Algorithm(v);
        setOutOfDate();
    }
}

void OpDWTForward::selectWavelet(int v)
{
    if ( m_waveletValue != Wavelet(v) ) {
        m_waveletValue = Wavelet(v);
        setOutOfDate();
    }
}

void OpDWTForward::selectOutputHDR(int v)
{
    if ( m_outputHDRValue != !!v ) {
        m_outputHDRValue = !!v;
        setOutOfDate();
    }
}
