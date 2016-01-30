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
#include "opcmycompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "algorithm.h"
#include "console.h"
#include "cielab.h"

static Photo
blackDot()
{
    return Photo(Magick::Image(Magick::Geometry(1,1), Magick::Color(0,0,0)), Photo::Linear);
}

class WorkerCMYCompose : public OperatorWorker {
public:
    WorkerCMYCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int l_count = m_inputs[0].count();
        int c_count = m_inputs[1].count();
        int m_count = m_inputs[2].count();
        int y_count = m_inputs[3].count();
        int photo_count = qMax(qMax(qMax(l_count, c_count), m_count), y_count);

        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pLuminance;
            Photo pCyan;
            Photo pMagenta;
            Photo pYellow;

            if ( l_count )
                pLuminance = m_inputs[0][i%l_count];
            else
                pLuminance = blackDot();

            if ( c_count )
                pCyan = m_inputs[1][i%c_count];
            else
                pCyan = blackDot();

            if ( m_count )
                pMagenta = m_inputs[2][i%m_count];
            else
                pMagenta = blackDot();

            if ( y_count )
                pYellow = m_inputs[3][i%y_count];
            else
                pYellow = blackDot();

            Magick::Image& iLuminance = pLuminance.image();
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();

            unsigned w = qMax(qMax(qMax(iLuminance.columns(),iCyan.columns()), iMagenta.columns()), iYellow.columns());
            unsigned h = qMax(qMax(qMax(iLuminance.rows(),iCyan.rows()), iMagenta.rows()), iYellow.rows());
            if ( iLuminance.columns() != w || iLuminance.rows() != h )
                iLuminance.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            if ( iCyan.columns() != w || iCyan.rows() != h )
                iCyan.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            if ( iMagenta.columns() != w || iMagenta.rows() != h )
                iMagenta.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            if ( iYellow.columns() != w || iYellow.rows() != h )
                iYellow.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);

            try {
                Magick::Pixels iCyan_cache(iCyan);
                Magick::Pixels iMagenta_cache(iMagenta);
                Magick::Pixels iYellow_cache(iYellow);
                Magick::Pixels iLuminance_cache(iLuminance);


                Photo photo(Photo::Linear);
                photo.createImage(w, h);
                photo.setSequenceNumber(i);
                photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
                photo.setTag(TAG_NAME, "LCMY Composition");
                ResetImage(photo.image());
                Magick::Pixels iPhoto_cache(photo.image());
                Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
                int line = 0;
#pragma omp parallel for dfl_threads(4, iCyan, iMagenta, iYellow, iLuminance)
                for ( int y = 0 ; y < int(h) ; ++y ) {
                    const Magick::PixelPacket *pxl_Cyan = iCyan_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Magenta = iMagenta_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Yellow = iYellow_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Luminance = iLuminance_cache.getConst(0, y, w, 1);
                    if ( m_error || !pxl_Cyan || !pxl_Magenta || !pxl_Yellow || !pxl_Luminance ) {
                        if (!m_error)
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( unsigned x = 0 ; x < w ; ++x ) {
                        quantum_t rgb[3];
                        quantum_t cyan = 0;
                        quantum_t magenta = 0;
                        quantum_t yellow = 0;
                        if ( pxl_Cyan ) cyan = (pxl_Cyan[x].green+pxl_Cyan[x].blue)/2;
                        if ( pxl_Magenta ) magenta = (pxl_Magenta[x].red+pxl_Magenta[x].blue)/2;
                        if ( pxl_Yellow) yellow = (pxl_Yellow[x].red+pxl_Yellow[x].green)/2;

                        rgb[0] =  - cyan + magenta + yellow;
                        rgb[1] =    cyan - magenta + yellow;
                        rgb[2] =    cyan + magenta - yellow;
                        if ( l_count ) {
                            double lum = LUMINANCE(pxl_Luminance?pxl_Luminance[x].red:0,
                                                   pxl_Luminance?pxl_Luminance[x].green:0,
                                                   pxl_Luminance?pxl_Luminance[x].blue:0);
                            double cur = LUMINANCE(rgb[0],
                                                   rgb[1],
                                                   rgb[2]);
                            double mul = lum/cur;
                            rgb[0] = DF_ROUND(mul*rgb[0]);
                            rgb[1] = DF_ROUND(mul*rgb[1]);
                            rgb[2] = DF_ROUND(mul*rgb[2]);
                        }
                        pxl[y*w+x].red = clamp(rgb[0]);
                        pxl[y*w+x].green = clamp(rgb[1]);
                        pxl[y*w+x].blue = clamp(rgb[2]);
                    }
#pragma omp critical
                    {
                        emitProgress(i, photo_count,line++, h);
                    }
                }
                iPhoto_cache.sync();
                outputPush(0, photo);
                emitProgress(i, photo_count, 1, 1);
            }
            catch (std::exception &e) {
                if (l_count)
                    setError(pLuminance, e.what());
                if (c_count)
                    setError(pCyan, e.what());
                if (m_count)
                    setError(pMagenta, e.what());
                if (y_count)
                    setError(pYellow, e.what());
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

OpCMYCompose::OpCMYCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "LCMY Compose"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput(tr("Luminance"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Cyan"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Magenta"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Yellow"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("RGB"), this));
}

OpCMYCompose *OpCMYCompose::newInstance()
{
    return new OpCMYCompose(m_process);
}

OperatorWorker *OpCMYCompose::newWorker()
{
    return new WorkerCMYCompose(m_thread, this);
}
