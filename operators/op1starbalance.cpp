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
#include "op1starbalance.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "photo.h"
#include "console.h"
#include "hdr.h"
#include "cielab.h"
#include "whitebalance.h"

template<class T> void normalizeRGB(T *rgb) {
    qreal l = LUMINANCE(rgb[0], rgb[1], rgb[2]);
    rgb[0] = rgb[0]/l;
    rgb[1] = rgb[1]/l;
    rgb[2] = rgb[2]/l;
}

class Worker1StarBalance : public OperatorWorker {
public:
    Worker1StarBalance(qreal bv, qreal epsilonR, qreal epsilonG, qreal epsilonB, qreal temp, qreal tint,
                       bool outputHDR,
                       QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_bv(bv),
        m_epsilonR(epsilonR),
        m_epsilonG(epsilonG),
        m_epsilonB(epsilonB),
        m_temp(temp),
        m_tint(tint),
        m_outputHDR(outputHDR)
    {
        if (m_epsilonR < 1)
            m_epsilonR = -1./m_epsilonR;
        if (m_epsilonG < 1)
            m_epsilonG = -1./m_epsilonG;
        if (m_epsilonB < 1)
            m_epsilonB = -1./m_epsilonB;
    }

    Photo process(const Photo &, int, int) { throw 0; }

    void play() {
        int p = 0,
                c = m_inputs[0].count();
        foreach(Photo photo, m_inputs[0]) {
            if (aborted())
                continue;
            QVector<QPointF> points = photo.getPoints();
            if ( points.size() < 2 ) {
                dflError("Needs at least two points, one on a star and the others on the background");
                continue;
            }
            Photo targetPhoto(photo);
            //Photo photoBackground(photo);
            //Photo photoBalance(photo);
            //Photo photoEpsilon(photo);

            qreal starLum = 0;
            qreal star[3] = {};
            qreal background_i[3] = {};
            qreal *background = &background_i[0]; // indirectio for macos
            foreach(QPointF point, points) {
                QVector<qreal> prgb = photo.pixelColor(point.x(), point.y());
                qreal pl = LUMINANCE(prgb[0], prgb[1], prgb[2]);
                if (pl > starLum) {
                    starLum = pl;
                    star[0] = prgb[0];
                    star[1] = prgb[1];
                    star[2] = prgb[2];
                }
                background[0] += prgb[0];
                background[1] += prgb[1];
                background[2] += prgb[2];
            }
            background[0] = (background[0]-star[0])/(points.size()-1);
            background[1] = (background[1]-star[1])/(points.size()-1);
            background[2] = (background[2]-star[2])/(points.size()-1);

            //B-V to T
            qreal idealStarTemperature =
                    4600 * (1 / (.92 * m_bv + 1.7) + 1 / (.92 * m_bv + .62));
            qreal idealStarRGB[3];
            WhiteBalance::Temperature_to_RGB(idealStarTemperature,idealStarRGB);
            idealStarRGB[1] *= m_tint;

            qreal whitePoint[3];
            WhiteBalance::Temperature_to_RGB(m_temp, whitePoint);

            qreal  epsilonColor_i[3] = {m_epsilonR, m_epsilonG, m_epsilonB};
            qreal *epsilonColor = &epsilonColor_i[0]; // indirection for macos

            qDebug("S[%f,%f,%f",star[0],star[1],star[2]);
            normalizeRGB(idealStarRGB);
            normalizeRGB(star);
            normalizeRGB(whitePoint);
            //compute lum for normalized star
            starLum = LUMINANCE(star[0],star[1],star[2]);
            qreal idealStarLum = LUMINANCE(idealStarRGB[0],
                                           idealStarRGB[1],
                                           idealStarRGB[2]);
            // delta * (A-G) = $(A') * L(A-G)
            qreal delta_i[3];
            qreal *delta = &delta_i[0]; //indirection for macos
            for (int c = 0; c < 3; ++c) {
                delta[c] = (idealStarRGB[c]/star[c]) * starLum/idealStarLum
                          * whitePoint[c];
                //delta[c] = 1;
            }
            qDebug("NS[%f,%f,%f",star[0],star[1],star[2]);
            qDebug("G[%f,%f,%f]",background[0],background[1],background[2]);
            bool hdr = targetPhoto.getScale() == Photo::HDR;
            Magick::Image &srcImage = photo.image();
            Magick::Image &image = targetPhoto.image();
            ResetImage(image);
            int h = image.rows(),
                    w = image.columns();
            std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
            std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
            dfl_block bool error = false;
            dfl_parallel_for(y, 0, h, 4, (srcImage, image), {
                                 Magick::PixelPacket *pixels = pixel_cache->get(0, y, w, 1);
                                 const Magick::PixelPacket *src = pixel_cache->getConst(0, y, w, 1);
                                 if (error || !pixels || !src)  {
                                     if (!error)
                                     dflError(DF_NULL_PIXELS);
                                     error = true;
                                     continue;
                                 }
                                 for (int x = 0; x < w; ++x) {
                                     qreal a[3];
                                     if (hdr) {
                                         a[0] = fromHDR(src[x].red);
                                         a[1] = fromHDR(src[x].green);
                                         a[2] = fromHDR(src[x].blue);
                                     }
                                     else {
                                         a[0] = src[x].red;
                                         a[1] = src[x].green;
                                         a[2] = src[x].blue;
                                     }
                                     qreal t[3];
                                     for (int c = 0; c < 3; ++c) {
                                         /*
                                         if ( a[c] - background[c] > 0 ) {
                                             t[c] = delta[c] * (a[c]  - background[c] )
                                                    + epsilonColor[c];
                                         } else if (a[c]  - background[c] + epsilonColor[c] > 0) {
                                             t[c] = delta[c] * (a[c]  - background[c] + epsilonColor[c] )
                                                    ;
                                         } else {
                                             t[c] = delta[c] * a[c] - background[c] + epsilonColor[c];
                                         }
                                                                      */
                                         t[c] = (a[c] * delta[c]) - (background[c] * delta[c]) + epsilonColor[c];
                                     }
                                     if (m_outputHDR) {
                                         pixels[x].red = toHDR(t[0]);
                                         pixels[x].green = toHDR(t[1]);
                                         pixels[x].blue = toHDR(t[2]);
                                     } else {
                                         pixels[x].red = clamp<quantum_t>(t[0]);
                                         pixels[x].green = clamp<quantum_t>(t[1]);
                                         pixels[x].blue = clamp<quantum_t>(t[2]);
                                     }
                                 }
                                 (void)m_temp;
                                 (void)m_tint;
                                 pixel_cache->sync();
                             });

            if (m_outputHDR) {
                targetPhoto.setScale(Photo::HDR);
            } else {
                targetPhoto.setScale(Photo::Linear);
            }
            outputPush(0, targetPhoto);
            //outputPush(1, photoBackground);
            //outputPush(2, photoBalance);
            //outputPush(3, photoEpsilon);
            emitProgress(p, c, 1, 1);
            ++p;
        }
        if (aborted()) {
            emitFailure();
        }  else {
            emitSuccess();
        }
    }

private:
    qreal m_bv;
    qreal m_epsilonR;
    qreal m_epsilonG;
    qreal m_epsilonB;
    qreal m_temp;
    qreal m_tint;
    bool m_outputHDR;
};


Op1StarBalance::Op1StarBalance(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator","1 Star Balance"), Operator::All, parent),
    m_bv(new OperatorParameterSlider("B-V", tr("B-V"), tr("B-V"),
                                     Slider::Value, Slider::Linear, Slider::Real,
                                     -.33, 1.4, 0.656, -.33, 1.4, Slider::FilterNothing,this)),
    m_epsilonRed(new OperatorParameterSlider("epsilonRed", tr("Ɛ Red"), tr("1 Star Balance Ɛ Red"),
                                          Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                          1./QuantumRange, QuantumRange, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_epsilonGreen(new OperatorParameterSlider("epsilonGreen", tr("Ɛ Green"), tr("1 Star Balance Ɛ Green"),
                                          Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                          1./QuantumRange, QuantumRange, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_epsilonBlue(new OperatorParameterSlider("epsilonBlue", tr("Ɛ Blue"), tr("1 Star Balance Ɛ Blue"),
                                          Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                          1./QuantumRange, QuantumRange, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_targetTemperature(new OperatorParameterSlider("temperature", tr("Temperature"), tr("White Point Temperature"),
                                                    Slider::Value, Slider::Logarithmic, Slider::Integer,
                                                    2000, 12000, 6500, 2000, 12000, Slider::FilterNothing,this)),
    m_targetTint(new OperatorParameterSlider("tint", tr("Green tint"), tr("White Point Green Tint"),
                                             Slider::Percent, Slider::Logarithmic, Slider::Real,
                                             M_SQRT1_2, M_SQRT2, 1, 0.01, 100, Slider::FilterNothing, this)),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(outputHDR(int)))),
    m_outputHDRValue(false)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addOutput(new OperatorOutput(tr("Background"), this));
    addOutput(new OperatorOutput(tr("Balance"), this));
    addOutput(new OperatorOutput(tr("Ɛ"), this));

    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);
    addParameter(m_bv);
    addParameter(m_epsilonRed);
    addParameter(m_epsilonGreen);
    addParameter(m_epsilonBlue);
    addParameter(m_targetTemperature);
    addParameter(m_targetTint);
    addParameter(m_outputHDR);
}

Op1StarBalance *Op1StarBalance::newInstance()
{
    return new Op1StarBalance(m_process);
}

OperatorWorker *Op1StarBalance::newWorker()
{
    return new Worker1StarBalance(m_bv->value(),
                                  m_epsilonRed->value(),
                                  m_epsilonGreen->value(),
                                  m_epsilonBlue->value(),
                                  m_targetTemperature->value(),
                                  m_targetTint->value(),
                                  m_outputHDRValue,
                                  m_thread, this);
}

void Op1StarBalance::outputHDR(int v)
{
    if (m_outputHDRValue != !!v) {
        m_outputHDRValue = !!v;
        setOutOfDate();
    }
}
