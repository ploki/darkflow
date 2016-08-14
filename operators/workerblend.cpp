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
#include "workerblend.h"
#include <Magick++.h>
#include "algorithm.h"
#include "hdr.h"
#include "console.h"

using Magick::Quantum;

typedef float real;

WorkerBlend::WorkerBlend(OpBlend::BlendMode mode1, OpBlend::BlendMode mode2, bool outputHDR, QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_mode1(mode1),
    m_mode2(mode2),
    m_outputHDR(outputHDR)
{}

Photo WorkerBlend::process(const Photo &, int, int)
{
    throw 0;
}

template<typename PIXEL>
static inline void
blend(WorkerBlend *self, PIXEL *rgb, const PIXEL *p, OpBlend::BlendMode mode)
{
    switch ( mode ) {
    case OpBlend::Multiply:
        //f(a,b) = a*b
        rgb[0] = (real(rgb[0]) * real(p[0])) / QuantumRange;
        rgb[1] = (real(rgb[1]) * real(p[1])) / QuantumRange;
        rgb[2] = (real(rgb[2]) * real(p[2])) / QuantumRange;
        break;
    case OpBlend::Screen:
        //f(a,b) = 1-(1-a)(1-b)
        rgb[0] = QuantumRange-(1.-real(rgb[0])/QuantumRange)*(1.-real(p[0])/QuantumRange)*QuantumRange;
        rgb[1] = QuantumRange-(1.-real(rgb[1])/QuantumRange)*(1.-real(p[1])/QuantumRange)*QuantumRange;
        rgb[2] = QuantumRange-(1.-real(rgb[2])/QuantumRange)*(1.-real(p[2])/QuantumRange)*QuantumRange;
        break;
    case OpBlend::Overlay:
        //f(a,b) = a<.5 ? 2ab : 1-2(1-a)(1-b)
        self->dflError("Not Implemented");
        break;
    case OpBlend::HardLight:
        self->dflError("Not Implemented");
        break;
    case OpBlend::SoftLight:
        self->dflError("Not Implemented");
        break;
    case OpBlend::DivideBrighten: {
        //f(a,b) = a*(1-b)
        real mul[3] = {
            p[0]!=0?real(QuantumRange)/p[0]:0,
            p[1]!=0?real(QuantumRange)/p[1]:0,
            p[2]!=0?real(QuantumRange)/p[2]:0
        };
        rgb[0] = mul[0]*rgb[0];
        rgb[1] = mul[1]*rgb[1];
        rgb[2] = mul[2]*rgb[2];
    }
        break;
    case OpBlend::Divide: {
        //f(a,b) = a*(1-b)
        real mul[3] = {
            p[0]!=0?real(QuantumRange)/p[0]:0,
            p[1]!=0?real(QuantumRange)/p[1]:0,
            p[2]!=0?real(QuantumRange)/p[2]:0
        };
        real max = 0;
        if ( mul[0] > max) max = mul[0];
        if ( mul[1] > max) max = mul[1];
        if ( mul[2] > max) max = mul[2];
        mul[0]/=max;
        mul[1]/=max;
        mul[2]/=max;
        rgb[0] = mul[0]*rgb[0];
        rgb[1] = mul[1]*rgb[1];
        rgb[2] = mul[2]*rgb[2];
    }
        break;
    case OpBlend::DivideDarken: {
        //f(a,b) = a*(1-b)
        real mul[3] = {
            (p[0]!=0)?real(1)/p[0]:0,
            (p[1]!=0)?real(1)/p[1]:0,
            (p[2]!=0)?real(1)/p[2]:0
        };
        rgb[0] = mul[0]*rgb[0];
        rgb[1] = mul[1]*rgb[1];
        rgb[2] = mul[2]*rgb[2];
    }
        break;
    case OpBlend::Addition:
        rgb[0] = rgb[0]+p[0];
        rgb[1] = rgb[1]+p[1];
        rgb[2] = rgb[2]+p[2];
        break;
    case OpBlend::Subtract:
        rgb[0] = rgb[0]-p[0];
        rgb[1] = rgb[1]-p[1];
        rgb[2] = rgb[2]-p[2];
        break;
    case OpBlend::Difference: {
        PIXEL r = rgb[0]-p[0];
        PIXEL g = rgb[1]-p[1];
        PIXEL b = rgb[2]-p[2];
        if ( r < 0 ) r=-r;
        if ( g < 0 ) g=-g;
        if ( b < 0 ) b=-b;
        rgb[0] = r;
        rgb[1] = g;
        rgb[2] = b;
        break;
    }
    case OpBlend::DarkenOnly:
        rgb[0] = qMin(rgb[0], p[0]);
        rgb[1] = qMin(rgb[1], p[1]);
        rgb[2] = qMin(rgb[2], p[2]);
        break;
    case OpBlend::LightenOnly:
        rgb[0] = qMax(rgb[0], p[0]);
        rgb[1] = qMax(rgb[1], p[1]);
        rgb[2] = qMax(rgb[2], p[2]);
        break;
    }
}

template<typename PIXEL>
static inline void
blend(WorkerBlend *self,
      Magick::PixelPacket *a,
      bool aHdr,
      const Magick::PixelPacket *b,
      bool bHdr,
      const Magick::PixelPacket *c,
      bool cHdr,
      Magick::PixelPacket *u,
      Magick::PixelPacket *o,
      OpBlend::BlendMode mode1,
      OpBlend::BlendMode mode2,
      bool outputHDR)
{
    PIXEL rgb[3] = {0, 0, 0};
    if ( a ) {
        if (!aHdr) { rgb[0] = a->red; rgb[1] = a->green; rgb[2] = a->blue; }
        else { rgb[0] = fromHDR(a->red); rgb[1] = fromHDR(a->green); rgb[2] = fromHDR(a->blue); }
    }
    if ( b ) {
        PIXEL p[3];
        if (!bHdr) { p[0] = b->red; p[1] = b->green; p[2] = b->blue; }
        else { p[0] = fromHDR(b->red); p[1] = fromHDR(b->green); p[2] = fromHDR(b->blue); }
        blend<PIXEL>(self, rgb, p, mode1);
    }
    if ( c ) {
        PIXEL p[3];
        if (!cHdr) { p[0] = c->red; p[1] = c->green; p[2] = c->blue; }
        else { p[0] = fromHDR(c->red); p[1] = fromHDR(c->green); p[2] = fromHDR(c->blue); }
        blend<PIXEL>(self, rgb, p, mode2);
    }
    if (u) {
        if ( rgb[0] < 0 ) u->red = -rgb[0]; else u->red = 0;
        if ( rgb[1] < 0 ) u->green = -rgb[1]; else u->green = 0;
        if ( rgb[2] < 0 ) u->blue = -rgb[2]; else u->blue = 0;
    }
    if (o) {
        if ( rgb[0] > QuantumRange ) o->red = clamp(rgb[0]-QuantumRange); else o->red = 0;
        if ( rgb[1] > QuantumRange ) o->green = clamp(rgb[1]-QuantumRange); else o->green = 0;
        if ( rgb[2] > QuantumRange ) o->blue = clamp(rgb[2]-QuantumRange); else o->blue = 0;
    }
    if ( outputHDR ) {
        a->red = clamp(toHDR(rgb[0]));
        a->green = clamp(toHDR(rgb[1]));
        a->blue = clamp(toHDR(rgb[2]));
    }
    else {
        a->red = clamp(rgb[0]);
        a->green = clamp(rgb[1]);
        a->blue = clamp(rgb[2]);
    }
}

void WorkerBlend::play()
{
    int a_count = m_inputs[0].count();
    int b_count = m_inputs[1].count();
    int c_count = m_inputs[2].count();

    int complete = qMin(1,qMax(b_count, c_count)) * m_inputs[0].count();
    int n = 0;
    foreach(Photo photo, m_inputs[0]) {
        if (aborted())
            continue;
        Photo photoA(photo);
        Photo *photoB = NULL;
        Photo *photoC = NULL;
        if ( b_count )
            photoB = &m_inputs[1][n%b_count];
        if ( c_count )
            photoC = &m_inputs[2][n%c_count];

        Photo underflow(photo);
        Photo overflow(photo);
        Magick::Image& srcImage = photo.image();
        Magick::Image *imageA = &photoA.image();
        Magick::Image *imageB = NULL;
        Magick::Image *imageC = NULL;

        Ordinary::Pixels *imageA_cache = NULL;
        Ordinary::Pixels *imageB_cache = NULL;
        Ordinary::Pixels *imageC_cache = NULL;
        try {
            Q_ASSERT(NULL != imageA);
            ResetImage(*imageA);
            ResetImage(underflow.image());
            ResetImage(overflow.image());

            std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
            std::shared_ptr<Ordinary::Pixels> underflow_cache(new Ordinary::Pixels(underflow.image()));
            std::shared_ptr<Ordinary::Pixels> overflow_cache(new Ordinary::Pixels(overflow.image()));
            imageA_cache = new Ordinary::Pixels(*imageA);
            int w = srcImage.columns();
            int h = srcImage.rows();
            int b_w = 0;
            int b_h = 0;
            int c_w = 0;
            int c_h = 0;
            if ( photoB ) {
                imageB = &photoB->image();
                imageB_cache = new Ordinary::Pixels(*imageB);
                b_w = imageB->columns();
                b_h = imageB->rows();
            }
            if ( photoC ) {
                imageC = &photoC->image();
                imageC_cache = new Ordinary::Pixels(*imageC);
                c_w = imageC->columns();
                c_h = imageC->rows();
            }
            dfl_block int line = 0;
            bool aHDR = photoA.getScale() == Photo::HDR;
            bool bHDR = photoB && photoB->getScale() == Photo::HDR;
            bool cHDR = photoC && photoC->getScale() == Photo::HDR ;
            bool anyHDR = false;
            if ( m_outputHDR || aHDR || bHDR || cHDR)
                anyHDR = true;
            dfl_parallel_for(y, 0, h, 4, (srcImage, imageA?*imageA:Magick::Image(), imageB?*imageB:Magick::Image(), imageC?*imageC:Magick::Image(), overflow.image(), underflow.image()), {
                if ( m_error || aborted() )
                    continue;
                const Magick::PixelPacket *src = src_cache->get(0, y, w, 1);
                Magick::PixelPacket *pxl_u = underflow_cache->get(0, y, w, 1);
                Magick::PixelPacket *pxl_o = overflow_cache->get(0, y, w, 1);
                Magick::PixelPacket *pxl_A = imageA_cache->get(0, y, w, 1);
                const Magick::PixelPacket *pxl_B = NULL;
                const Magick::PixelPacket *pxl_C = NULL;
                if ( imageB_cache )
                    pxl_B = imageB_cache->getConst(0, y%b_h, b_w, 1);
                if ( imageC_cache )
                    pxl_C = imageC_cache->getConst(0, y%c_h, c_w, 1);
                if ( m_error || !src || !pxl_u || !pxl_o || !pxl_A || (imageB_cache && !pxl_B) || (imageC_cache && !pxl_C) ) {
                    if ( !m_error )
                        dflError(DF_NULL_PIXELS);
                    continue;
                }
                for ( int x = 0 ; x < w ; ++x ) {
                    const Magick::PixelPacket *pB = NULL;
                    const Magick::PixelPacket *pC = NULL;
                    if ( pxl_B ) pB = pxl_B+(x%b_w);
                    if ( pxl_C ) pC = pxl_C+(x%c_w);
                    pxl_A[x].red = src[x].red;
                    pxl_A[x].green = src[x].green;
                    pxl_A[x].blue = src[x].blue;
                    if (anyHDR)
                        blend<real>(this,
                                    pxl_A+x, aHDR,
                                    pB, bHDR,
                                    pC, cHDR,
                                    pxl_u+x,
                                    pxl_o+x,
                                    m_mode1,
                                    m_mode2,
                                    m_outputHDR);
                    else
                        blend<quantum_t>(this,
                                         pxl_A+x, aHDR,
                                         pB, bHDR,
                                         pC, cHDR,
                                         pxl_u+x,
                                         pxl_o+x,
                                         m_mode1,
                                         m_mode2,
                                         m_outputHDR);
                }
                imageA_cache->sync();
                underflow_cache->sync();
                overflow_cache->sync();
                dfl_critical_section(
                {
                    if ( line % 100 == 0)
                        emitProgress(n, a_count, line, h);
                    ++line;
                });
            });

            if ( b_h == 1 && b_w == 1 &&
                 c_h == 1 && c_w == 1 ) {
                Magick::Image & srcCurve(photoA.curve());
                ResetImage(photoA.curve());
                Ordinary::Pixels src_cache(srcCurve);
                Ordinary::Pixels curve_cache(photoA.curve());
                const Magick::PixelPacket *src = src_cache.getConst(0, 0, 65536, 1);
                Magick::PixelPacket *pxl_A = curve_cache.get(0, 0, 65536, 1);
                const Magick::PixelPacket *pxl_B = NULL;
                const Magick::PixelPacket *pxl_C = NULL;
                if ( imageB_cache )
                    pxl_B = imageB_cache->getConst(0, 0, 1, 1);
                if ( imageC_cache )
                    pxl_C = imageC_cache->getConst(0, 0, 1, 1);
                if ( m_error || !src || !pxl_A || ( imageB_cache && !pxl_B ) || ( imageC_cache && ! pxl_C) ) {
                    if ( !m_error )
                        dflError(DF_NULL_PIXELS);
                    continue;
                }
                for ( int x = 0; x < 65536 ; ++x ) {
                    pxl_A[x].red = src[x].red;
                    pxl_A[x].green = src[x].green;
                    pxl_A[x].blue = src[x].blue;
                    if (anyHDR)
                        blend<real>(this,
                                    pxl_A+x, (photoA.getScale() == Photo::HDR),
                                    pxl_B, (photoB?(photoB->getScale() == Photo::HDR): false),
                                    pxl_C, (photoC?(photoC->getScale() == Photo::HDR): false),
                                    NULL, NULL, m_mode1, m_mode2, m_outputHDR);
                    else
                        blend<quantum_t>(this,
                                    pxl_A+x, (photoA.getScale() == Photo::HDR),
                                    pxl_B, (photoB?(photoB->getScale() == Photo::HDR): false),
                                    pxl_C, (photoC?(photoC->getScale() == Photo::HDR): false),
                                    NULL, NULL, m_mode1, m_mode2, m_outputHDR);
                }
                curve_cache.sync();
            }
            if ( m_outputHDR )
                photoA.setScale(Photo::HDR);
            else if ( photoA.getScale() == Photo::HDR )
                photoA.setScale(Photo::Linear);
            overflow.setScale(Photo::Linear);
            underflow.setScale(Photo::Linear);
            outputPush(0, photoA);
            outputPush(1, overflow);
            outputPush(2, underflow);
        }
        catch (std::exception &e) {
            setError(photoA, e.what());
            if (photoB)
                setError(*photoB, e.what());
            if (photoC)
                setError(*photoC, e.what());
        }
        delete imageA_cache;
        delete imageB_cache;
        delete imageC_cache;
        emitProgress(n,complete, 1, 1);
        ++n;
    }
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
}

