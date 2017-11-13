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
#include "opbayercompose.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"

class WorkerBayerCompose : public OperatorWorker {
public:
    WorkerBayerCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int c0 = m_inputs[0].count(),
            c1 = m_inputs[1].count(),
            c2 = m_inputs[2].count(),
            c3 = m_inputs[3].count();
        if ( c0 != c1 || c0 != c2 || c0 != c3) {
            dflError("Not the same number of images on inputs");
            emitFailure();
            return;
        }
        for (int i = 0 ; i < c0 ; ++i) {
            Photo& p0 = m_inputs[0][i];
            Photo& p1 = m_inputs[1][i];
            Photo& p2 = m_inputs[2][i];
            Photo& p3 = m_inputs[3][i];
            Photo output(p0);
            Magick::Image& i0 = p0.image();
            Magick::Image& i1 = p1.image();
            Magick::Image& i2 = p2.image();
            Magick::Image& i3 = p3.image();
            int w = i0.columns();
            int h = i0.rows();
            if ( w != int(i1.columns()) || w != int(i2.columns()) || w != int(i3.columns()) ||
                 h != int(i1.rows()) || h != int(i2.rows()) || h != int(i3.rows())) {
                dflError("Images don't share the same geometry");
                emitFailure();
                return;
            }
            w*=2;
            h*=2;
            output.createImage(w, h);
            Magick::Image& dstImage = output.image();
            std::shared_ptr<Ordinary::Pixels> i0Cache(new Ordinary::Pixels(i0));
            std::shared_ptr<Ordinary::Pixels> i1Cache(new Ordinary::Pixels(i1));
            std::shared_ptr<Ordinary::Pixels> i2Cache(new Ordinary::Pixels(i2));
            std::shared_ptr<Ordinary::Pixels> i3Cache(new Ordinary::Pixels(i3));
            std::shared_ptr<Ordinary::Pixels> dstCache(new Ordinary::Pixels(dstImage));
            dfl_parallel_for(y, 0, h, 4, (dstImage, i0, i1, i2, i3), {
                                 Magick::PixelPacket *pixels = dstCache->get(0, y, w, 1);
                                 const Magick::PixelPacket *pixA;
                                 const Magick::PixelPacket *pixB;
                                 if (y % 2 == 0) {
                                     pixA = i0Cache->getConst(0, y/2, w/2, 1);
                                     pixB = i1Cache->getConst(0, y/2, w/2, 1);
                                 } else {
                                     pixA = i2Cache->getConst(0, y/2, w/2, 1);
                                     pixB = i3Cache->getConst(0, y/2, w/2, 1);
                                 }
                                 for (int x = 0 ; x < w ; ++x) {
                                     if (x % 2 == 0) {
                                         pixels[x] = pixA[x/2];
                                     } else {
                                         pixels[x] = pixB[x/2];
                                     }
                                 }
                                 dstCache->sync();
                             });
            outputPush(0, output);
            emitProgress(i, c0, 1, 1);
        }
        emitSuccess();
    }
};

OpBayerCompose::OpBayerCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Bayer compose"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("⬉"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("⬈"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("⬋"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("⬊"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
}

OpBayerCompose *OpBayerCompose::newInstance()
{
    return new OpBayerCompose(m_process);
}

OperatorWorker *OpBayerCompose::newWorker()
{
    return new WorkerBayerCompose(m_thread, this);
}
