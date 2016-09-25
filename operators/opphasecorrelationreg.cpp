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
#include "opphasecorrelationreg.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "operatorparameterdropdown.h"
#include "operatorparameterslider.h"
#include "discretefouriertransform.h"
#include "cielab.h"


class WorkerPhaseCorrelation : public OperatorWorker {
    DiscreteFourierTransform::WindowFunction m_window;
    double m_opening;
public:
    WorkerPhaseCorrelation(DiscreteFourierTransform::WindowFunction window,
                           double opening,
                           QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_window(window),
        m_opening(opening)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }

    void play() {
        int count = m_inputs[0].count();
        DiscreteFourierTransform *dftB = nullptr;
        Photo *refPhoto = Photo::findReference(m_inputs[0]);

        for (int i = 0 ; i < count ; ++i) {
            if (NULL == dftB) {
                Magick::Image imB = refPhoto ? refPhoto->image() : m_inputs[0][i].image();
                Photo::Gamma scale = refPhoto ? refPhoto->getScale() : m_inputs[0][i].getScale();
                imB = DiscreteFourierTransform::window(imB, scale, m_window, m_opening);
                imB = DiscreteFourierTransform::roll(imB, imB.columns()/2, imB.rows()/2);
                dftB = new DiscreteFourierTransform(imB, scale);
                dftB->conj();
            }
            Magick::Image imA = m_inputs[0][i].image();
            imA = DiscreteFourierTransform::window(imA, m_inputs[0][i].getScale(), m_window, m_opening);
            imA = DiscreteFourierTransform::roll(imA, imA.columns()/2, imA.rows()/2);
            DiscreteFourierTransform dftA(imA, m_inputs[0][i].getScale());
            dftA *= (*dftB);
            DiscreteFourierTransform denom(dftA);
            denom.abs();
            dftA /= denom;
            Magick::Image img = dftA.reverse(1, DiscreteFourierTransform::ReverseMagnitude);
            img = DiscreteFourierTransform::roll(img, img.columns()/2, img.rows()/2);
            int w = img.columns();
            int h = img.rows();
            double max = 0;
            double mx = 0, my = 0;
            Ordinary::Pixels cache(img);
            for (int y = 0 ; y < h ; ++y) {
                const Magick::PixelPacket *pixels = cache.getConst(0, y, w, 1);
                for (int x = 0 ; x < w ; ++x ) {
                    quantum_t lm = LUMINANCE_PIXEL(pixels[x]);
                    if ( lm > max ) {
                        max = lm;
                        mx = x;
                        my = y;
                    }
                }
            }
#define THRESHOLD (.25)
#define SPREAD 4
            int x1 = 0, x2 = 0;
            int radius;
            bool extinctionFound = false;
            const Magick::PixelPacket *pixels = cache.getConst(0, my, w, 1);
            for ( int x = 0 ; true ; ++x ) {
                x1 = mx-x;
                x2 = mx+x;
                if ( x1 < 0 || x2 >= w )
                    break;
                double l1 = LUMINANCE_PIXEL(pixels[x1]);
                double l2 = LUMINANCE_PIXEL(pixels[x1]);
                if ( l1 < THRESHOLD * max && l2 < THRESHOLD * max ) {
                    extinctionFound = true;
                    radius = SPREAD * (x2 - x1) / 2;
                    break;
                }
            }
            if (extinctionFound) {
                double totalLum = 0;
                double totalX = 0;
                double totalY = 0;
                for (int y = my-radius ; y < my+radius ; ++y) {
                    if ( y < 0 || y >= h )
                        continue;
                    const Magick::PixelPacket *pixels = cache.getConst(0, y, w, 1);
                    for (int x = mx-radius ; x < mx+radius ; ++x) {
                        if ( x < 0 || x >= w )
                            continue;
                        double l = LUMINANCE_PIXEL(pixels[x]);
                        totalLum +=l;
                        totalX += ( l * x );
                        totalY += ( l * y );
                    }
                }
                mx = totalX / totalLum;
                my = totalY / totalLum;
            }
            Photo registered(m_inputs[0][i]);
            QVector<QPointF> points;
            points.push_back(QPointF(mx, my));
            registered.setPoints(points);
            Photo newPhoto(img, Photo::Linear);
            newPhoto.setIdentity(m_operator->uuid()+":c:"+QString::number(i));
            newPhoto.setTag(TAG_NAME, registered.getTag(TAG_NAME));
            newPhoto.setPoints(points);
            outputPush(0, registered);
            outputPush(1, newPhoto);
            emitProgress(i, count, 0, 1);
        }
        delete dftB;
        emitSuccess();
    }
};

OpPhaseCorrelationReg::OpPhaseCorrelationReg(Process *parent) :
    Operator(OP_SECTION_REGISTRATION, QT_TRANSLATE_NOOP("Operator","Phase Correlation"), Operator::All, parent),
    m_window(new OperatorParameterDropDown("window", tr("Window"), this, SLOT(selectWindow(int)))),
    m_windowValue(DiscreteFourierTransform::WindowHamming),
    m_opening(new OperatorParameterSlider("opening", tr("Opening"), tr("Window Function - Opening"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .5, 0, 1, Slider::FilterPercent, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addOutput(new OperatorOutput(tr("Correlation"), this));

    m_window->addOption(DF_TR_AND_C("None"), DiscreteFourierTransform::WindowNone, false);
    m_window->addOption(DF_TR_AND_C("Hamming"), DiscreteFourierTransform::WindowHamming, true);
    m_window->addOption(DF_TR_AND_C("Hann"), DiscreteFourierTransform::WindowHann, false);
    m_window->addOption(DF_TR_AND_C("Nuttal"), DiscreteFourierTransform::WindowNuttal, false);
    m_window->addOption(DF_TR_AND_C("Blackman-Nuttal"), DiscreteFourierTransform::WindowBlackmanNuttal, false);
    m_window->addOption(DF_TR_AND_C("Blackman-Harris"), DiscreteFourierTransform::WindowBlackmanHarris, false);
    addParameter(m_window);
    addParameter(m_opening);
}

OpPhaseCorrelationReg *OpPhaseCorrelationReg::newInstance()
{
    return new OpPhaseCorrelationReg(m_process);
}

OperatorWorker *OpPhaseCorrelationReg::newWorker()
{
    return new WorkerPhaseCorrelation(DiscreteFourierTransform::WindowFunction(m_windowValue),
                                      m_opening->value(),
                                      m_thread, this);
}

void OpPhaseCorrelationReg::selectWindow(int v)
{
    if (m_windowValue != v) {
        m_windowValue = v;
        setOutOfDate();
    }
}
