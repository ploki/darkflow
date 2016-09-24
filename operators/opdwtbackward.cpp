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
#include "opdwtbackward.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "operatorparameterslider.h"
#include "hdr.h"

using Magick::Quantum;

class WorkerDWTBackward : public OperatorWorker {
    int m_planes;
    QVector<double> m_coefs;
    double m_luminosity;
    bool m_outputHDR;
public:
    WorkerDWTBackward(int planes,
                      QVector<double> coefs,
                      double luminosity,
                      bool outputHDR,
                      QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_planes(planes),
        m_coefs(coefs),
        m_luminosity(luminosity),
        m_outputHDR(outputHDR)
    {}
    Photo process(const Photo &, int, int) {
        throw 0;
    }
    void play() {
        Triplet<double> *img = nullptr;
        int w, h;
        int count = 0;
        int signCount=m_inputs[m_planes].count();
        for (int i = 0 ; i < m_planes ; ++i)
            count = qMax(count, m_inputs[i].count());
        for (int i = 0 ; i < count ; ++i ) {
            for ( int j = 0 ; j < m_planes ; ++j ) {
                if (m_inputs[j].count() == 0)
                    continue;
                std::shared_ptr<Ordinary::Pixels> signCache(nullptr);
                Magick::Image *signImage = nullptr;
                if (signCount != 0) {
                    signImage = &m_inputs[m_planes][(i*m_planes+j)%signCount].image();
                    signCache.reset(new Ordinary::Pixels(*signImage));
                }
                Photo planePhoto = m_inputs[j][i%m_inputs[j].count()];
                Magick::Image& planeImage = planePhoto.image();
                std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(planeImage));
                int c_w = planeImage.columns(),
                    c_h = planeImage.rows();
                if ( !img ) {
                    w = c_w;
                    h = c_h;
                    img = new Triplet<double>[w*h];
                }
                else if ( ( w != c_w || h != c_h ) &&
                          ( c_w != 1 && c_h != 1 ) ) {
                    dflError(tr("Size mismatch"));
                    continue;
                }
                const Magick::PixelPacket *signPixels = nullptr;
                bool hdr = planePhoto.getScale() == Photo::HDR;
                if (signCache)
                    signPixels = signCache->getConst(0, 0, w, h);
                dfl_parallel_for(y, 0, h, 4, (planeImage), {
                                     const Magick::PixelPacket *pixels = cache->getConst(0, y%c_h, c_w, 1);
                                     for (int x = 0 ; x < w ; ++x) {
                                         Triplet<double> pixel;
                                         if (hdr) {
                                             pixel.red = fromHDR(pixels[x%c_w].red);
                                             pixel.green = fromHDR(pixels[x%c_w].green);
                                             pixel.blue = fromHDR(pixels[x%c_w].blue);
                                         }
                                         else {
                                             pixel.red = pixels[x%c_w].red;
                                             pixel.green = pixels[x%c_w].green;
                                             pixel.blue = pixels[x%c_w].blue;
                                         }
                                         if (signPixels) {
                                             if (signPixels[y*w+x].red)
                                                 pixel.red *= -1;
                                             if (signPixels[y*w+x].green)
                                                 pixel.green *= -1;
                                             if (signPixels[y*w+x].blue)
                                                 pixel.blue *= -1;
                                         }
                                         pixel *= m_coefs[j];
                                         img[y*w+x] += pixel;
                                     }
                                 });
            }
            Photo output(m_outputHDR ? Photo::HDR : Photo::Linear);
            output.setIdentity(m_operator->uuid()+QString(":%0").arg(i));
            output.setTag(TAG_NAME, tr("reconstruction"));
            output.createImage(w, h);
            Photo underflow(m_outputHDR ? Photo::HDR : Photo::Linear);
            underflow.setIdentity(m_operator->uuid()+QString(":u:%0").arg(i));
            underflow.setTag(TAG_NAME, tr("underflow"));
            underflow.createImage(w, h);
            Photo overflow(m_outputHDR ? Photo::HDR : Photo::Linear);
            overflow.setIdentity(m_operator->uuid()+QString(":o:%0").arg(i));
            overflow.setTag(TAG_NAME, tr("overflow"));
            overflow.createImage(w, h);
            std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(output.image()));
            std::shared_ptr<Ordinary::Pixels> uCache(new Ordinary::Pixels(underflow.image()));
            std::shared_ptr<Ordinary::Pixels> oCache(new Ordinary::Pixels(overflow.image()));
            dfl_parallel_for(y, 0, h, 4, (output.image(), underflow.image(), overflow.image()), {
                                 Magick::PixelPacket *pixel = cache->get(0, y, w, 1);
                                 Magick::PixelPacket *u = uCache->get(0, y, w, 1);
                                 Magick::PixelPacket *o = oCache->get(0, y, w, 1);
                                 for (int x = 0 ; x < w ; ++x) {
                                     img[y*w+x] *= m_luminosity;
                                     if (m_outputHDR) {
                                         if (img[y*w+x].red < 0) u[x].red = toHDR(-img[y*w+x].red);
                                         if (img[y*w+x].green < 0) u[x].green = toHDR(-img[y*w+x].green);
                                         if (img[y*w+x].blue < 0) u[x].blue = toHDR(-img[y*w+x].blue);
                                         if (img[y*w+x].red > QuantumRange ) o[x].red = toHDR(img[y*w+x].red-QuantumRange);
                                         if (img[y*w+x].green > QuantumRange ) o[x].green = toHDR(img[y*w+x].green-QuantumRange);
                                         if (img[y*w+x].blue > QuantumRange ) o[x].blue = toHDR(img[y*w+x].blue-QuantumRange);
                                         pixel[x].red = toHDR(img[y*w+x].red);
                                         pixel[x].green = toHDR(img[y*w+x].green);
                                         pixel[x].blue = toHDR(img[y*w+x].blue);
                                     }
                                     else {
                                         if (img[y*w+x].red < 0) u[x].red = clamp<quantum_t>(-img[y*w+x].red);
                                         if (img[y*w+x].green < 0) u[x].green = clamp<quantum_t>(-img[y*w+x].green);
                                         if (img[y*w+x].blue < 0) u[x].blue = clamp<quantum_t>(-img[y*w+x].blue);
                                         if (img[y*w+x].red > QuantumRange ) o[x].red = clamp<quantum_t>(img[y*w+x].red-QuantumRange);
                                         if (img[y*w+x].green > QuantumRange ) o[x].green = clamp<quantum_t>(img[y*w+x].green-QuantumRange);
                                         if (img[y*w+x].blue > QuantumRange ) o[x].blue = clamp<quantum_t>(img[y*w+x].blue-QuantumRange);
                                         pixel[x].red = clamp<quantum_t>(img[y*w+x].red);
                                         pixel[x].green = clamp<quantum_t>(img[y*w+x].green);
                                         pixel[x].blue = clamp<quantum_t>(img[y*w+x].blue);
                                     }
                                 }
                             });
            delete img; img = 0;
            outputPush(0, output);
            outputPush(1, overflow);
            outputPush(2, underflow);
        }
        emitSuccess();
    }
};

OpDWTBackward::OpDWTBackward(int nPlanes, Process *parent) :
    Operator(OP_SECTION_FREQUENCY_DOMAIN,
             QT_TRANSLATE_NOOP("Operator", "%0-way Backward DWT"),
             Operator::All, parent),
    m_planes(nPlanes),
    m_coefs(nPlanes),
    m_luminosity(new OperatorParameterSlider("luminosity", tr("Luminosity"), tr("Deconvolution Luminosity"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 4, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this)),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(selectOutputHDR(int)))),
    m_outputHDRValue(false)
{
    m_classIdentifier = m_classIdentifier.arg(m_planes);
    m_name = m_name.arg(m_planes);

    for (int i = 1 ; i <= m_planes ; ++i) {
        QString name = tr("Plane %0").arg(i);
        addInput(new OperatorInput(name, OperatorInput::Set, this));

        name = QString("multiplier:%0").arg(i);
        m_coefs[i-1] = new OperatorParameterSlider(name.toLocal8Bit().data(),
                                                 tr("Plane %0").arg(i),
                                                 tr("Discrete Wavelet Reconstruction - PLane %0 multiplier").arg(i),
                                                 Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                                 .5, 2., 1, 1./QuantumRange, QuantumRange,
                                                 Slider::FilterExposureFromOne, this);
        addParameter(m_coefs[i-1]);
    }
    addParameter(m_luminosity);
    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);
    addParameter(m_outputHDR);

    addInput(new OperatorInput(tr("Sign"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addOutput(new OperatorOutput(tr("Overflow"), this));
    addOutput(new OperatorOutput(tr("Underflow"), this));
}

OpDWTBackward *OpDWTBackward::newInstance()
{
    return new OpDWTBackward(m_planes, m_process);
}

OperatorWorker *OpDWTBackward::newWorker()
{
    QVector<double> coefs;
    for (int i = 0 ; i < m_planes ; ++i ) {
        coefs.push_back(m_coefs[i]->value());
    }
    return new WorkerDWTBackward(m_planes, coefs, m_luminosity->value(), m_outputHDRValue, m_thread, this);
}

void OpDWTBackward::selectOutputHDR(int v)
{
    if ( m_outputHDRValue != !!v ) {
        m_outputHDRValue = !!v;
        setOutOfDate();
    }
}
