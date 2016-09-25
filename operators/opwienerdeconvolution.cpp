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
#include "opwienerdeconvolution.h"
#include "workerwienerdeconvolution.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"


OpWienerDeconvolution::OpWienerDeconvolution(Process *parent) :
    Operator(OP_SECTION_FREQUENCY_DOMAIN, QT_TRANSLATE_NOOP("Operator", "Wiener Deconvolution"), Operator::NonHDR, parent),
    m_luminosity(new OperatorParameterSlider("luminosity", tr("Luminosity"), tr("Wiener Deconvolution Luminosity"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 4, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this)),
    m_snr(new OperatorParameterSlider("snr", tr("SNR"), tr("Wiener Deconvolution SNR"), Slider::Value, Slider::Logarithmic, Slider::Real, 1, 10000, 1000, 1, 100000, Slider::FilterPixels, this)),
    m_iterations(new OperatorParameterSlider("iterations", tr("Iterations"), tr("Wiener Deconvolution Iterations"), Slider::Value, Slider::Linear, Slider::Integer, 1, 100, 1, 1, 100000, Slider::FilterPixels, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Kernel"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_luminosity);
    addParameter(m_snr);
    addParameter(m_iterations);
}

OpWienerDeconvolution *OpWienerDeconvolution::newInstance()
{
    return new OpWienerDeconvolution(m_process);
}

OperatorWorker *OpWienerDeconvolution::newWorker()
{
    return new WorkerWienerDeconvolution(m_luminosity->value(), m_snr->value(), m_iterations->value(), m_thread, this);
}
