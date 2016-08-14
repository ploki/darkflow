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
#include "oprgbdecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "console.h"
#include "cielab.h"

class WorkerRGBDecompose : public OperatorWorker {
public:
    WorkerRGBDecompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int p = 0,
                c = m_inputs[0].count();
        foreach(Photo photo, m_inputs[0]) {
            if (aborted())
                continue;
            Photo pRed(photo);
            Photo pGreen(photo);
            Photo pBlue(photo);
            Photo pLuminance(photo);
            Magick::Image& srcImage = photo.image();
            Magick::Image& iRed = pRed.image();
            Magick::Image& iGreen = pGreen.image();
            Magick::Image& iBlue = pBlue.image();
            Magick::Image& iLuminance = pLuminance.image();

            try {
                ResetImage(iRed);
                ResetImage(iGreen);
                ResetImage(iBlue);
                ResetImage(iLuminance);
                Ordinary::Pixels src_cache(srcImage);
                Ordinary::Pixels iRed_cache(iRed);
                Ordinary::Pixels iGreen_cache(iGreen);
                Ordinary::Pixels iBlue_cache(iBlue);
                Ordinary::Pixels iLuminance_cache(iLuminance);
                int w = srcImage.columns();
                int h = srcImage.rows();
                int line = 0;
#pragma omp parallel for dfl_threads(4, srcImage, iRed, iGreen, iBlue, iLuminance)
                for ( int y = 0 ; y < h ; ++y ) {
                    const Magick::PixelPacket *src = src_cache.getConst(0, y, w, 1);
                    Magick::PixelPacket *pxl_Red = iRed_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Green = iGreen_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Blue = iBlue_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Luminance = iLuminance_cache.get(0, y, w, 1);
                    if ( m_error || !src || !pxl_Red || !pxl_Green || !pxl_Blue || !pxl_Luminance ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( int x = 0 ; x < w ; ++x ) {
                        pxl_Red[x].red = pxl_Red[x].green = pxl_Red[x].blue = src[x].red;
                        pxl_Green[x].red = pxl_Green[x].green = pxl_Green[x].blue = src[x].green;
                        pxl_Blue[x].red = pxl_Blue[x].green = pxl_Blue[x].blue = src[x].blue;
                        pxl_Luminance[x].red =
                        pxl_Luminance[x].green =
                        pxl_Luminance[x].blue = DF_ROUND(LUMINANCE_PIXEL(src[x]));
                    }
                    iRed_cache.sync();
                    iGreen_cache.sync();
                    iBlue_cache.sync();
                    iLuminance_cache.sync();
#pragma omp critical
                    {
                        emitProgress(p, c, line++, h);
                    }
                }
                outputPush(0, pLuminance);
                outputPush(1, pRed);
                outputPush(2, pGreen);
                outputPush(3, pBlue);
                emitProgress(p, c, 1, 1);
                ++p;
            }
            catch(std::exception &e) {
                setError(pRed, e.what());
                emitFailure();
                return;
            }
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }
};

OpRGBDecompose::OpRGBDecompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "LRGB Decompose"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Luminance"), this));
    addOutput(new OperatorOutput(tr("Red"), this));
    addOutput(new OperatorOutput(tr("Green"), this));
    addOutput(new OperatorOutput(tr("Blue"), this));
}

OpRGBDecompose *OpRGBDecompose::newInstance()
{
    return new OpRGBDecompose(m_process);
}

OperatorWorker *OpRGBDecompose::newWorker()
{
    return new WorkerRGBDecompose(m_thread, this);
}
