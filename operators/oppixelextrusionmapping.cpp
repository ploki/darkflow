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
#include "oppixelextrusionmapping.h"
#include "operatorparameterdropdown.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"
#include "hdr.h"
using Magick::Quantum;

class WorkerPixelExtrusionMapping : public OperatorWorker {
public:
    WorkerPixelExtrusionMapping(OpPixelExtrusionMapping::Scale scale, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_scale(scale)
    {}
    Photo process(const Photo &photo, int, int) {
        static const int fullHeight = 256;
        Photo newPhoto(photo);
        Magick::Image& srcImage = newPhoto.image();
        int w = srcImage.columns();
        int h = srcImage.rows();
        Magick::Image dstImage(Magick::Geometry(w+h/2, h+fullHeight), Magick::Color(0, 0, 0));
        Ordinary::Pixels srcCache(srcImage);
        Ordinary::Pixels dstCache(dstImage);
        Magick::PixelPacket *dstPixels = dstCache.get(0, 0, w+h/2, h+fullHeight);
        bool hdr = photo.getScale() == Photo::HDR;
        for (int y = 0 ; y < h ; ++y) {
            const Magick::PixelPacket *srcPixels = srcCache.getConst(0, y, w, 1);
            for ( int x = 0 ; x < w ; ++x ) {
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
                qreal k;
                switch (m_scale) {
                default:
                case OpPixelExtrusionMapping::Linear:
                    k= lum/QuantumRange;
                    break;
                case OpPixelExtrusionMapping::Gamma:
                    k = pow(lum/QuantumRange, 1./2.2);
                    break;
                case OpPixelExtrusionMapping::Log:
                    if (lum == 0)
                        k = 0;
                    else
                        k = log2(lum)/16;
                    break;
                }

                int height = fullHeight * k;
                if (height == 0 ) height = 1;
                for ( int yy = y + (fullHeight-height) ; yy < y+fullHeight; ++yy ) {
                    int xx = x+y/2 ;
                    int idx = yy*(w+h/2)+xx;
                    dstPixels[idx].red = srcPixels[x].red;
                    dstPixels[idx].green = srcPixels[x].green;
                    dstPixels[idx].blue = srcPixels[x].blue;
                }
            }
        }
        dstCache.sync();
        newPhoto.image() = dstImage;
        return newPhoto;
    }
private:
    OpPixelExtrusionMapping::Scale m_scale;
};

OpPixelExtrusionMapping::OpPixelExtrusionMapping(Process *parent) :
    Operator(OP_SECTION_ANALYSIS, QT_TRANSLATE_NOOP("Operator", "Pixel Extrusion Mapping"), Operator::All, parent),
    m_scale(new OperatorParameterDropDown("vScale", tr("Vertical Scale"),this, SLOT(setScale(int)))),
    m_scaleValue(Linear)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Extrusion"), this));

    m_scale->addOption(DF_TR_AND_C("Linear"), Linear, true);
    m_scale->addOption(DF_TR_AND_C("Gamma"), Gamma);
    m_scale->addOption(DF_TR_AND_C("Log"), Log);
    addParameter(m_scale);
}

OpPixelExtrusionMapping *OpPixelExtrusionMapping::newInstance()
{
    return new OpPixelExtrusionMapping(m_process);
}

OperatorWorker *OpPixelExtrusionMapping::newWorker()
{
    return new WorkerPixelExtrusionMapping(m_scaleValue, m_thread, this);
}

void OpPixelExtrusionMapping::setScale(int v)
{
    if (m_scaleValue != Scale(v)) {
        m_scaleValue = Scale(v);
        setOutOfDate();
    }
}
