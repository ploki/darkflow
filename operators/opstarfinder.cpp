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
#include "opstarfinder.h"
#include "operatorparameterslider.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "cielab.h"
#include "hdr.h"
#include "atrouswavelettransform.h"
#include <Magick++.h>

#include <vector>

using Magick::Quantum;

static inline double
luminance(bool hdr, const Magick::PixelPacket &pixel)
{
    if (hdr)
        return LUMINANCE(
                    fromHDR(pixel.red),
                    fromHDR(pixel.green),
                    fromHDR(pixel.blue));
    else
        return LUMINANCE_PIXEL(pixel);
}

class WorkerStarFinder : public OperatorWorker {
    double m_threshold;
public:
    WorkerStarFinder(double threshold, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_threshold(threshold)
    {}
    Photo process(const Photo& photo, int, int) {
        Photo srcPhoto(photo);
        Magick::Image &srcImage = srcPhoto.image();
        int w = srcImage.columns(),
            h = srcImage.rows();

        ATrousWaveletTransform dwt(srcPhoto, b3SplineWavelet, sizeof(b3SplineWavelet)/sizeof(*b3SplineWavelet), 0);
        Photo sign(srcPhoto);
        Photo highFreqs = dwt.transform(0, 2, srcPhoto.getScale(), sign);

        bool hdr = srcPhoto.getScale() == Photo::HDR;
        Ordinary::Pixels srcCache(highFreqs.image());
        Ordinary::Pixels signCache(sign.image());
        const Magick::PixelPacket *srcPixels = srcCache.getConst(0, 0, w, h);
        const Magick::PixelPacket *signPixels = signCache.getConst(0, 0, w, h);
        std::shared_ptr<QVector<QPointF> > bingo(new QVector<QPointF>);
        dfl_block unsigned int count = 0;
        static const int maxCount = 5000;
        double thresholdValue = m_threshold * QuantumRange;
        dfl_parallel_for(y, 1, (h-1), 4, (), {
                             if (count >= maxCount) {
                                 continue;
                             }
                             for (int x = 1 ; x < w-1 ; ++x ) {
                                 double lum = luminance(hdr, srcPixels[y*w+x]);
                                 double sp = luminance(false, signPixels[y*w+x]);
                                 if (0 != sp)
                                    lum = 0;
                                 if ( lum >= thresholdValue ) {
                                     bool matched = true;
                                     for (int yy = y-1 ; yy <= y+1 && matched ; ++yy) {
                                         for (int xx = x-1 ; xx <= x+1 && matched ; ++xx) {
                                             if ( xx == x && yy == y )
                                                continue;
                                             double oLum = luminance(hdr, srcPixels[yy*w+xx]);
                                             if ( oLum > lum )
                                                matched = false;
                                         }
                                     }
                                     if ( matched ) {
                                         if (count >= maxCount)
                                            break;
                                         dfl_critical_section(
                                            bingo->push_back(QPointF(x,y));
                                            atomic_incr(&count);
                                         );
                                     }
                                 }
                             }
                         });
        if (count >= maxCount) {
            dflWarning(tr("Too many stars found, consider lowering threshold"));
        }
        dflInfo(tr("Star Finder found %0 star(s)").arg(count));
        srcPhoto.setPoints(*bingo);
        return srcPhoto;
    }
};

OpStarFinder::OpStarFinder(Process *parent) :
    Operator(OP_SECTION_ANALYSIS, QT_TRANSLATE_NOOP("Operator", "Star Finder"), Operator::All, parent),
    m_threshold(new OperatorParameterSlider("threshold", tr("Threshold"), tr("Star Finder - Detection Threshold"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<8), 1, 1./(1<<4), 1./QuantumRange, 1, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Stars overlay"), this));
    addParameter(m_threshold);
}

OpStarFinder *OpStarFinder::newInstance()
{
    return new OpStarFinder(m_process);
}

OperatorWorker *OpStarFinder::newWorker()
{
    return new WorkerStarFinder(m_threshold->value(), m_thread, this);
}
