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
#include "opcolormap.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"

static bool pixelLowerThan(const Magick::PixelPacket& lhs, const Magick::PixelPacket& rhs)
{
    double ll = LUMINANCE_PIXEL(lhs);
    double rl = LUMINANCE_PIXEL(rhs);
    return (ll > rl);
}

class WorkerColorMap : public OperatorWorker {
public:
    WorkerColorMap(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo srcPhoto(photo);
        Photo newPhoto(photo);
        Magick::Image& srcImage = srcPhoto.image();
        Magick::Image& dstImage = newPhoto.image();
        int w = srcImage.columns();
        int h = srcImage.rows();
        ResetImage(dstImage);
        Ordinary::Pixels srcCache(srcImage);
        Ordinary::Pixels dstCache(dstImage);
        Magick::PixelPacket *dstPixels = dstCache.get(0, 0, w, h);
        for (int y = 0 ; y < h ; ++y) {
            const Magick::PixelPacket *srcPixels = srcCache.getConst(0, y, w, 1);
            for ( int x = 0 ; x < w ; ++x ) {
                int idx = y*w+x;
                dstPixels[idx].red = srcPixels[x].red;
                dstPixels[idx].green = srcPixels[x].green;
                dstPixels[idx].blue = srcPixels[x].blue;
            }
        }
        qSort(&dstPixels[0], &dstPixels[w*h], pixelLowerThan);
        dstCache.sync();
        newPhoto.image() = dstImage;
        return newPhoto;
    }
};

OpColorMap::OpColorMap(Process *parent) :
    Operator(OP_SECTION_ANALYSIS, QT_TRANSLATE_NOOP("Operator", "Color Map"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Color Map"), this));
}

OpColorMap *OpColorMap::newInstance()
{
    return new OpColorMap(m_process);
}

OperatorWorker *OpColorMap::newWorker()
{
    return new WorkerColorMap(m_thread, this);
}
