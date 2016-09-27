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
#include <Magick++.h>

#include <vector>

using Magick::Quantum;

static bool
anyFound(std::shared_ptr<std::vector<bool> > &m, int w, int x, int y) {
    return     (*m)[y*w+x]
            || (*m)[y*w+x-1]
            || (*m)[y*w+x+1]
            || (*m)[(y-1)*w+x]
            || (*m)[(y+1)*w+x]
            || (*m)[(y-1)*w+x-1]
            || (*m)[(y-1)*w+x+1]
            || (*m)[(y+1)*w+x-1]
            || (*m)[(y+1)*w+x+1];
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
        bool hdr = srcPhoto.getScale() == Photo::HDR;
        std::shared_ptr<Ordinary::Pixels> srcCache(new Ordinary::Pixels(srcImage));
        std::shared_ptr<std::vector<bool> > map(new std::vector<bool>(w*h));
        QVector<QPointF> bingo;
        dfl_block int count = 0;
        static const int maxCount = 5000;
        double thresholdValue = m_threshold * QuantumRange;
        dfl_parallel_for(y, 1, (h-1), 4, (srcImage), {
                             if (count >= maxCount) {
                                 continue;
                             }
                             const Magick::PixelPacket *srcPixels = srcCache->getConst(0, y, w, 1);
                             for (int x = 1 ; x < w-1 ; ++x ) {
                                 double lum;
                                 if (hdr) {
                                     lum = LUMINANCE(
                                                 fromHDR(srcPixels[x].red),
                                                 fromHDR(srcPixels[x].green),
                                                 fromHDR(srcPixels[x].blue));
                                 }
                                 else {
                                     lum = LUMINANCE_PIXEL(srcPixels[x]);
                                 }
                                 if ( lum > thresholdValue ) {
                                     if ( !anyFound(map, w, x, y) ) {
                                         if (count >= maxCount)
                                             break;
                                         dfl_critical_section(
                                         bingo.push_back(QPointF(x,y));
                                         atomic_incr(&count);
                                         );
                                     }
                                     (*map)[y*w+x] = true;
                                 }
                             }
                         });
        if (count >= maxCount) {
            dflWarning(tr("Too many stars found, consider lowering threshold"));
        }
        dflInfo(tr("Star Finder found %0 star(s)").arg(count));
        srcPhoto.setPoints(bingo);
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
