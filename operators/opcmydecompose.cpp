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
#include "opcmydecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "console.h"
#include "cielab.h"

class WorkerCMYDecompose : public OperatorWorker {
public:
    WorkerCMYDecompose(QThread *thread, Operator *op) :
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
            Photo pCyan(photo);
            Photo pMagenta(photo);
            Photo pYellow(photo);
            Photo pLuminance(photo);
            Magick::Image& srcImage = photo.image();
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();
            Magick::Image& iLuminance = pLuminance.image();

            try {
                ResetImage(iCyan);
                ResetImage(iMagenta);
                ResetImage(iYellow);
                ResetImage(iLuminance);
                Magick::Pixels src_cache(srcImage);
                Magick::Pixels iCyan_cache(iCyan);
                Magick::Pixels iMagenta_cache(iMagenta);
                Magick::Pixels iYellow_cache(iYellow);
                Magick::Pixels iLuminance_cache(iLuminance);
                int w = srcImage.columns();
                int h = srcImage.rows();
                int line = 0;
#pragma omp parallel for dfl_threads(4, srcImage, iCyan, iMagenta, iYellow, iLuminance)
                for ( int y = 0 ; y < h ; ++y ) {
                    const Magick::PixelPacket *src = src_cache.getConst(0, y, w, 1);
                    Magick::PixelPacket *pxl_Cyan = iCyan_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Magenta = iMagenta_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Yellow = iYellow_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Luminance = iLuminance_cache.get(0, y, w, 1);
                    if ( m_error || !src || !pxl_Cyan || !pxl_Magenta || !pxl_Yellow || !pxl_Luminance ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( int x = 0 ; x < w ; ++x ) {
                        pxl_Luminance[x].red =
                                pxl_Luminance[x].green =
                                pxl_Luminance[x].blue =
                                        DF_ROUND(LUMINANCE_PIXEL(src[x]));
                        pxl_Cyan[x].green = pxl_Cyan[x].blue = pxl_Cyan[x].red =
                                (quantum_t(src[x].green) + quantum_t(src[x].blue))/2;
                        pxl_Magenta[x].green = pxl_Magenta[x].blue = pxl_Magenta[x].red =
                                (quantum_t(src[x].red) + quantum_t(src[x].blue))/2;
                        pxl_Yellow[x].green = pxl_Yellow[x].blue = pxl_Yellow[x].red =
                                (quantum_t(src[x].red) + quantum_t(src[x].green))/2;
                    }
                    iCyan_cache.sync();
                    iMagenta_cache.sync();
                    iYellow_cache.sync();
                    iLuminance_cache.sync();
#pragma omp critical
                    {
                        emitProgress(p, c, line++, h);
                    }
                }
                outputPush(0, pLuminance);
                outputPush(1, pCyan);
                outputPush(2, pMagenta);
                outputPush(3, pYellow);
                emitProgress(p, c, 1, 1);
                ++p;
            }
            catch(std::exception &e) {
                setError(pCyan, e.what());
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

OpCMYDecompose::OpCMYDecompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "LCMY Decompose"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Luminance"), this));
    addOutput(new OperatorOutput(tr("Cyan"), this));
    addOutput(new OperatorOutput(tr("Magenta"), this));
    addOutput(new OperatorOutput(tr("Yellow"), this));

}

OpCMYDecompose *OpCMYDecompose::newInstance()
{
    return new OpCMYDecompose(m_process);
}

OperatorWorker *OpCMYDecompose::newWorker()
{
    return new WorkerCMYDecompose(m_thread, this);
}
