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

#include "opwindowfunction.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "operatorparameterdropdown.h"
#include "operatorparameterslider.h"
#include "discretefouriertransform.h"

using Magick::Quantum;

class WindowFunctionWorker : public OperatorWorker {
    DiscreteFourierTransform::WindowFunction m_window;
    double m_opening;
    bool m_keepBackground;
public:
    WindowFunctionWorker(DiscreteFourierTransform::WindowFunction window,
                         double opening, bool keepBackground,
                         QThread *thread,
                         Operator *op) :
        OperatorWorker(thread, op),
        m_window(window),
        m_opening(opening),
        m_keepBackground(keepBackground)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        Magick::Image& image = newPhoto.image();
        if (!m_keepBackground) {
            ResetImage(image);
            int w = image.columns(),
                h = image.rows();
            std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(image));
            dfl_parallel_for(y, 0, h, 4, (image), {
                                 Magick::PixelPacket *pixels = cache->get(0, y, w, 1);
                                 for (int x = 0 ; x < w ; ++x) {
                                     pixels[x].red = pixels[x].green = pixels[x].blue = QuantumRange;
                                 }
                             });

        }
        image = DiscreteFourierTransform::window(image, photo.getScale(), m_window, m_opening);
        return newPhoto;
    }
};

OpWindowFunction::OpWindowFunction(Process *parent) :
    Operator(OP_SECTION_MASK, QT_TRANSLATE_NOOP("Operator", "Window Function"), Operator::All, parent),
    m_window(new OperatorParameterDropDown("window", tr("Window"), this, SLOT(selectWindow(int)))),
    m_windowValue(DiscreteFourierTransform::WindowHamming),
    m_opening(new OperatorParameterSlider("opening", tr("Opening"), tr("Window Function - Opening"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .5, 0, 1, Slider::FilterPercent, this)),
    m_keepBackground(new OperatorParameterDropDown("keepBackground", tr("Keep Background"),this, SLOT(selectKeepBackground(int)))),
    m_keepBackgroundValue(true)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_window->addOption(DF_TR_AND_C("None"), DiscreteFourierTransform::WindowNone, false);
    m_window->addOption(DF_TR_AND_C("Hamming"), DiscreteFourierTransform::WindowHamming, true);
    m_window->addOption(DF_TR_AND_C("Hann"), DiscreteFourierTransform::WindowHann, false);
    m_window->addOption(DF_TR_AND_C("Nuttal"), DiscreteFourierTransform::WindowNuttal, false);
    m_window->addOption(DF_TR_AND_C("Blackman-Nuttal"), DiscreteFourierTransform::WindowBlackmanNuttal, false);
    m_window->addOption(DF_TR_AND_C("Blackman-Harris"), DiscreteFourierTransform::WindowBlackmanHarris, false);

    m_keepBackground->addOption(DF_TR_AND_C("No"), false);
    m_keepBackground->addOption(DF_TR_AND_C("Yes"), true, true);

    addParameter(m_window);
    addParameter(m_opening);
    addParameter(m_keepBackground);
}

OpWindowFunction *OpWindowFunction::newInstance()
{
    return new OpWindowFunction(m_process);
}

OperatorWorker *OpWindowFunction::newWorker()
{
    return new WindowFunctionWorker(DiscreteFourierTransform::WindowFunction(m_windowValue),
                                    m_opening->value(),
                                    m_keepBackgroundValue,
                                    m_thread, this);
}

void OpWindowFunction::selectWindow(int v)
{
    if (m_windowValue != v) {
        m_windowValue = v;
        setOutOfDate();
    }
}

void OpWindowFunction::selectKeepBackground(int v)
{
    if (m_keepBackgroundValue != !!v) {
        m_keepBackgroundValue = !!v;
        setOutOfDate();
    }
}

