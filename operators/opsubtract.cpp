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
#include "opsubtract.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "algorithm.h"
#include "console.h"
#include <Magick++.h>
using Magick::Quantum;


class WorkerSubtract : public OperatorWorker {

public:
    WorkerSubtract(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}

    void subtract(Magick::Image& minuend,
                  Magick::Image& subtrahend,
                  Magick::Image* addend,
                  Magick::Image& underflow) {
        int w = minuend.columns(),
                h = minuend.rows();
        int s_w = subtrahend.columns(),
                s_h = subtrahend.rows();
        if ( (w != s_w ||
              h != s_h) &&
             (s_w !=1 || s_h != 1 )) {
            dflError("size mismatch");
            return;
        }
        Magick::Image &srcImage(minuend);
        ResetImage(minuend);
        ResetImage(underflow);
        Ordinary::Pixels src_cache(srcImage);
        Ordinary::Pixels minuend_cache(minuend);
        Ordinary::Pixels subtrahend_cache(subtrahend);
        Ordinary::Pixels *addend_cache = NULL;
        if (addend)
            addend_cache = new Ordinary::Pixels(*addend);
        Ordinary::Pixels underflow_cache(underflow);
#pragma omp parallel for dfl_threads(4, srcImage, minuend, subtrahend, addend?*addend:Magick::Image())
        for ( int y = 0 ; y < h ; ++y ) {
            const Magick::PixelPacket *src = src_cache.getConst(0, y, w, 1);
            Magick::PixelPacket *minuend_pixels = minuend_cache.get(0, y, w, 1);
            Magick::PixelPacket *underflow_pixels = underflow_cache.get(0, y, w,1);
            const Magick::PixelPacket *subtrahend_pixels = subtrahend_cache.getConst(0, (1 == s_h ? 0 : y), s_w, 1);
            const Magick::PixelPacket *addend_pixels = NULL;
            if ( addend_cache )
                addend_pixels = addend_cache->getConst(0, (1 == s_h ? 0 : y), s_w, 1);
            if ( m_error || !src || !minuend_pixels || !underflow_pixels || !subtrahend_pixels || (addend_cache && !addend_pixels) ) {
                if ( !m_error )
                    dflError(DF_NULL_PIXELS);
                continue;
            }
            for ( int x = 0 ; x < w ; ++x ) {
                int s_x = ( 1 == s_w ) ? 0 : x ;
                quantum_t r = src[x].red - subtrahend_pixels[s_x].red;
                quantum_t g = src[x].green - subtrahend_pixels[s_x].green;
                quantum_t b = src[x].blue - subtrahend_pixels[s_x].blue;
                if ( addend_pixels ) {
                    r += addend_pixels[s_x].red;
                    g += addend_pixels[s_x].green;
                    b += addend_pixels[s_x].blue;
                }
                underflow_pixels[x].red = underflow_pixels[x].green = underflow_pixels[x].blue =
                        ( r < 0 || g < 0 || b < 0) ? QuantumRange : 0;
                minuend_pixels[x].red = clamp<quantum_t>(r, 0, QuantumRange);
                minuend_pixels[x].green = clamp<quantum_t>(g, 0, QuantumRange);
                minuend_pixels[x].blue = clamp<quantum_t>(b, 0, QuantumRange);
            }
            minuend_cache.sync();
            underflow_cache.sync();
        }
    }

    void play() {
        Q_ASSERT( m_inputs.count() == 3 );
        if ( m_inputs[1].count() == 0 )
            return OperatorWorker::play();
        int n_photos = m_inputs[0].count() * m_inputs[1].count();
        int n = 0;
        int n_sub = 0;
        foreach(Photo subtrahend, m_inputs[1]) {
            foreach(Photo minuend, m_inputs[0]) {
                ++n;
                if (aborted())
                    continue;
                Photo underflow(minuend);
                Magick::Image *addend_image=NULL;
                Magick::Image *addend_curve=NULL;
                if ( n_sub < m_inputs[2].count() ) {
                    addend_image = &m_inputs[2][n_sub].image();
                    addend_curve = &m_inputs[2][n_sub].curve();
                }
                try {
                    subtract(minuend.image(), subtrahend.image(), addend_image, underflow.image());
                    if ( subtrahend.image().columns() == 1 &&
                         subtrahend.image().rows() == 1 ) {
                        Photo dummy(minuend.curve(),Photo::Linear);
                        subtract(minuend.curve(), subtrahend.image(), addend_curve, dummy.image());
                    }
                }
                catch (std::exception &e) {
                    setError(subtrahend, e.what());
                    setError(minuend, e.what());
                    if (addend_image)
                        setError(m_inputs[2][n_sub], e.what());
                    emitFailure();
                    return;
                }

                outputPush(0, minuend);
                outputPush(1, underflow);
                emit progress(n, n_photos);
            }
            ++n_sub;
        }
        if ( aborted() )
            emitFailure();
        else
            emitSuccess();
    }

    Photo process(const Photo &photo, int, int) { return Photo(photo); }
};


OpSubtract::OpSubtract(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, QT_TRANSLATE_NOOP("Operator", "Subtract"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput(tr("Minuend"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Subtrahend"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Addend"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Difference"), this));
    addOutput(new OperatorOutput(tr("Underflow"), this));

}

OpSubtract *OpSubtract::newInstance()
{
    return new OpSubtract(m_process);
}

OperatorWorker *OpSubtract::newWorker()
{
    return new WorkerSubtract(m_thread, this);
}
