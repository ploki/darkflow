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
#include "colorfilter.h"

#include "photo.h"
#include "console.h"
#include <Magick++.h>

ColorFilter::ColorFilter(qreal r, qreal g, qreal b, QObject *parent) :
    Algorithm(true, parent),
    m_rgb()
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}

void ColorFilter::applyOnImage(Magick::Image &image, bool hdr)
{
    Magick::Image srcImage(image);
    ResetImage(image);
    int h = image.rows(),
            w = image.columns();
    std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
    std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
    dfl_block_array(qreal, rgb, 3);
    if (hdr) {
        rgb[0] = log2(m_rgb[0])*4096;
        rgb[1] = log2(m_rgb[1])*4096;
        rgb[2] = log2(m_rgb[2])*4096;
    }
    else {
        rgb[0] = m_rgb[0];
        rgb[1] = m_rgb[1];
        rgb[2] = m_rgb[2];
    }
    dfl_block bool error=false;
    dfl_parallel_for(y, 0, h, 4, (image, srcImage), {
        Magick::PixelPacket *pixels = pixel_cache->get(0,y,w,1);
        const Magick::PixelPacket *src = src_cache->getConst(0,y,w,1);
        if ( error || !pixels || !src ) {
            if (!error)
                dflError(DF_NULL_PIXELS);
            error=true;
            continue;
        }
        for (int x = 0 ; x < w ; ++x ) {
            using Magick::Quantum;
            if (hdr) {
                pixels[x].red=clamp<double>(src[x].red+rgb[0],0,QuantumRange);
                pixels[x].green=clamp<double>(src[x].green+rgb[1],0,QuantumRange);
                pixels[x].blue=clamp<double>(src[x].blue+rgb[2],0,QuantumRange);
            }
            else {
                pixels[x].red=clamp<double>(src[x].red*rgb[0],0,QuantumRange);
                pixels[x].green=clamp<double>(src[x].green*rgb[1],0,QuantumRange);
                pixels[x].blue=clamp<double>(src[x].blue*rgb[2],0,QuantumRange);
            }
        }
        pixel_cache->sync();
    });
}
