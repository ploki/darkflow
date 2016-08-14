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
#include "lutbased.h"
#include "console.h"
#include <Magick++.h>

using Magick::Quantum;

LutBased::LutBased(QObject *parent) :
    Algorithm(true, parent),
    m_lut(new quantum_t[QuantumRange+1]),
    m_hdrLut(new quantum_t[QuantumRange+1])
{

}

LutBased::~LutBased()
{
    delete[] m_hdrLut;
    delete[] m_lut;
}

void LutBased::applyOnImage(Magick::Image &image, bool hdr)
{
    Magick::Image srcImage(image);
    ResetImage(image);
    quantum_t *lut = hdr ? m_hdrLut : m_lut;
    int h = image.rows(),
            w = image.columns();
    std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
    std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
    dfl_block bool error=false;
    dfl_parallel_for(y, 0, h, 4, (image, srcImage), {
                     Magick::PixelPacket *pixels = pixel_cache->get(0,y,w,1);
                     const Magick::PixelPacket *src = src_cache->getConst(0,y,w,1);
                     if ( error || !pixels || !src ) {
                         if ( !error )
                         dflError(DF_NULL_PIXELS);
                         error=true;
                         continue;
                     }
                     for ( int x = 0 ; x < w ; ++x ) {
                         pixels[x].red=lut[src[x].red];
                         pixels[x].green=lut[src[x].green];
                         pixels[x].blue=lut[src[x].blue];
                     }
                     pixel_cache->sync();

                 });
}

quantum_t LutBased::applyOnQuantum(quantum_t v, bool hdr)
{
    return (hdr ? m_hdrLut : m_lut)[clamp(v)];
}

