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
#include "oprepair.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "operatorparameterslider.h"

class WorkerRepair : public OperatorWorker {
    qreal m_radius;
public:
    WorkerRepair(qreal radius, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_radius(radius)
    {
    }
    Photo process(const Photo &, int, int) {
        throw 0;
    }
    void play() {
        int c0 = m_inputs[0].count();
        int c1 = m_inputs[1].count();
        for (int i = 0 ; i < c0 ; ++i) {
            Photo& srcPhoto = m_inputs[0][i];
            Photo& mapPhoto = m_inputs[1][i%c1];
            Photo blurredPhoto(srcPhoto);
            Photo outputPhoto(srcPhoto);
            blurredPhoto.image().gaussianBlur(m_radius, m_radius);
            Magick::Image &srcImage = srcPhoto.image();
            Magick::Image &mapImage = mapPhoto.image();
            Magick::Image &blurredImage = blurredPhoto.image();
            Magick::Image &outputImage = outputPhoto.image();
            ResetImage(outputImage);
            int w = srcImage.columns();
            int h = srcImage.rows();
            if ( w != int(mapImage.columns()) || h != int(mapImage.rows())) {
                dflError(tr("Input image and pixels map differ in size"));
                emitFailure();
                return;
            }
            std::shared_ptr<Ordinary::Pixels> srcCache(new Ordinary::Pixels(srcImage));
            std::shared_ptr<Ordinary::Pixels> mapCache(new Ordinary::Pixels(mapImage));
            std::shared_ptr<Ordinary::Pixels> blurredCache(new Ordinary::Pixels(blurredImage));
            std::shared_ptr<Ordinary::Pixels> outputCache(new Ordinary::Pixels(outputImage));
            dfl_parallel_for(y, 0, h, 4, (srcImage, mapImage, blurredImage, outputImage), {
                                 const Magick::PixelPacket *srcPixels = srcCache->getConst(0, y, w, 1);
                                 const Magick::PixelPacket *mapPixels = mapCache->getConst(0, y, w, 1);
                                 const Magick::PixelPacket *blurredPixels = blurredCache->getConst(0, y, w, 1);
                                 Magick::PixelPacket *outputPixels = outputCache->get(0, y, w, 1);
                                 for (int x = 0 ; x < w ; ++x) {
                                     if (mapPixels[x].red) {
                                         outputPixels[x] = blurredPixels[x];
                                     } else {
                                         outputPixels[x] = srcPixels[x];
                                     }
                                 }
                                outputCache->sync();
                             });
            emitProgress(i, c0, 1, 1);
            outputPush(0, outputPhoto);
        }
        emitSuccess();
    }
};

OpRepair::OpRepair(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Repair"), Operator::All, parent),
    m_radius(new OperatorParameterSlider("radius", tr("Radius"), tr("Repair radius"), Slider::Value, Slider::Linear, Slider::Real, .1, 100, 5, .1, 1000, Slider::FilterPixels, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Bad pixels map"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_radius);
}

OpRepair *OpRepair::newInstance()
{
    return new OpRepair(m_process);
}

OperatorWorker *OpRepair::newWorker()
{
    return new WorkerRepair(m_radius->value(), m_thread, this);
}
