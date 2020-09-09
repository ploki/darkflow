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
#include "opbayerdecompose.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"

class WorkerBayerDecompose : public OperatorWorker {
public:
    WorkerBayerDecompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo& photo, int, int) {
        Magick::Image& srcImage = const_cast<Magick::Image&>(photo.image());
        int w = srcImage.columns();
        int h = srcImage.rows();
        if (w % 2) {
            w-=1;
            dflWarning(tr("Image witdth is odd"));
        }
        if (h % 2) {
            h-=1;
            dflWarning(tr("Image height is odd"));
        }
        Photo p0(photo),
              p1(photo),
              p2(photo),
              p3(photo);
        int hh = h/2, ww = w/2;
        p0.createImage(ww, hh);
        p1.createImage(ww, hh);
        p2.createImage(ww, hh);
        p3.createImage(ww, hh);
        Magick::Image& i0 = p0.image();
        Magick::Image& i1 = p1.image();
        Magick::Image& i2 = p2.image();
        Magick::Image& i3 = p3.image();
        std::shared_ptr<Ordinary::Pixels> srcCache(new Ordinary::Pixels(srcImage));
        std::shared_ptr<Ordinary::Pixels> i0Cache(new Ordinary::Pixels(i0));
        std::shared_ptr<Ordinary::Pixels> i1Cache(new Ordinary::Pixels(i1));
        std::shared_ptr<Ordinary::Pixels> i2Cache(new Ordinary::Pixels(i2));
        std::shared_ptr<Ordinary::Pixels> i3Cache(new Ordinary::Pixels(i3));
        dfl_parallel_for(y, 0, hh, 4, (srcImage, i0, i1, i2, i3), {
                             const Magick::PixelPacket *src0 = srcCache->getConst(0, y*2, w, 1);
                             const Magick::PixelPacket *src1 = srcCache->getConst(0, y*2+1, w, 1);
                             Magick::PixelPacket *pix0 = i0Cache->get(0, y, ww, 1);
                             Magick::PixelPacket *pix1 = i1Cache->get(0, y, ww, 1);
                             Magick::PixelPacket *pix2 = i2Cache->get(0, y, ww, 1);
                             Magick::PixelPacket *pix3 = i3Cache->get(0, y, ww, 1);
                             for (int x = 0 ; x < ww ; ++x) {
                                 pix0[x] = src0[x*2];
                                 pix1[x] = src0[x*2+1];
                                 pix2[x] = src1[x*2];
                                 pix3[x] = src1[x*2+1];
                             }
                             i0Cache->sync();
                             i1Cache->sync();
                             i2Cache->sync();
                             i3Cache->sync();
                         });
        outputPush(3, p3);
        outputPush(2, p2);
        outputPush(1, p1);
        return        p0;
    }
};

OpBayerDecompose::OpBayerDecompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Bayer decompose"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("⬉"), this));
    addOutput(new OperatorOutput(tr("⬈"), this));
    addOutput(new OperatorOutput(tr("⬋"), this));
    addOutput(new OperatorOutput(tr("⬊"), this));
}

OpBayerDecompose *OpBayerDecompose::newInstance()
{
    return new OpBayerDecompose(m_process);
}

OperatorWorker *OpBayerDecompose::newWorker()
{
    return new WorkerBayerDecompose(m_thread, this);
}
