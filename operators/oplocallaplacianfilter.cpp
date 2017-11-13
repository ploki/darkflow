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
#include "oplocallaplacianfilter.h"
#include "workerlocallaplacianfilter.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

using Magick::Quantum;


OpLocalLaplacianFilter::OpLocalLaplacianFilter(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Local Laplacian Filter"),Operator::All, parent),
    m_alpha(new OperatorParameterSlider("alpha", tr("Alpha"), tr("Local Laplacian Filter alpha"), Slider::Value, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./(QuantumRange), QuantumRange, Slider::FilterLogarithmic, this)),
    m_beta(new OperatorParameterSlider("beta", tr("Beta"), tr("Local Laplacian Filter beta"), Slider::Value, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./(QuantumRange), QuantumRange, Slider::FilterLogarithmic, this)),
    m_sigma(new OperatorParameterSlider("sigma", tr("Sigma"), tr("Local Laplacian Filter sigma"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 0.5, 0, 1, Slider::FilterNothing, this)),
    m_startLevel(new OperatorParameterSlider("startLevel", tr("Start level"), tr("Local Laplacian Filter start level"), Slider::Value, Slider::Linear, Slider::Integer, 0, 5, 0, 0, 16, Slider::FilterNothing, this)),
    m_levelsCount(new OperatorParameterSlider("levelsCount", tr("Levels count"), tr("Local Laplacian Filter levels count"), Slider::Value, Slider::Linear, Slider::Integer, 0, 10, 5, 0, 16, Slider::FilterNothing, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_alpha);
    addParameter(m_beta);
    addParameter(m_sigma);
    addParameter(m_startLevel);
    addParameter(m_levelsCount);
}

OpLocalLaplacianFilter *OpLocalLaplacianFilter::newInstance()
{
    return new OpLocalLaplacianFilter(m_process);
}

OperatorWorker *OpLocalLaplacianFilter::newWorker()
{
    return new WorkerLocalLaplacianFilter(m_alpha->value(),
                                          m_beta->value(),
                                          m_sigma->value(),
                                          m_startLevel->value(),
                                          m_levelsCount->value(),
                                          m_thread, this);
}
