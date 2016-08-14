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
#include <cmath>
#include "desaturateshadows.h"
#include "photo.h"
#include <Magick++.h>
#include "cielab.h"
#include "hdr.h"
#include "ports.h"
#include "console.h"
using Magick::Quantum;

DesaturateShadows::DesaturateShadows(qreal highlightLimit,
                                     qreal range,
                                     qreal saturation,
                                     QObject *parent) :
    Algorithm(false, parent),
    m_lut(new double[QuantumRange+1]),
    m_highlightLimit(highlightLimit),
    m_range(range),
    m_saturation(saturation)
{
    double high = 16.L + log2(highlightLimit);
    double low = high - log2(range);
    double threshold_high = highlightLimit * QuantumRange;
    double threshold_low =  threshold_high / range;
    dfl_parallel_for(i, 0, int(QuantumRange+1), 1024, (), {
        if ( i < threshold_low ) {
            m_lut[i]=saturation;
        }
        else if ( i > threshold_high ) {
            m_lut[i]=1.;
        }
        else {
            //lut[i] = (sin(M_PI/(low-high)*(high+log(i/double(QuantumRange))/log(2))+M_PI/2.)+1.)/2.;
            m_lut[i] = ( saturation - 1. ) * (1.-(sin(M_PI/(low-high)*(high+log(i/double(QuantumRange))/log(2))+M_PI/2.)+1.)/2.) + 1.;
        }
    });
}

DesaturateShadows::~DesaturateShadows()
{
    delete[] m_lut;
}

void DesaturateShadows::applyOnImage(Magick::Image& image, bool hdr)
{
    Magick::Image srcImage(image);
    ResetImage(image);
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
            double rgb[3];
            if (hdr) {
                rgb[0]=fromHDR(src[x].red);
                rgb[1]=fromHDR(src[x].green);
                rgb[2]=fromHDR(src[x].blue);
            }
            else {
                rgb[0]=src[x].red;
                rgb[1]=src[x].green;
                rgb[2]=src[x].blue;
            }
            double lab[3];
            RGB_to_LinearLab(rgb,lab);
            quantum_t L= DF_ROUND(lab[0]*QuantumRange);
            if ( L > QuantumRange ) L=QuantumRange;
            if ( ! DF_EQUALS(m_lut[L],1.,.00001) )
            {
                lab[1]*=m_lut[L];
                lab[2]*=m_lut[L];
                LinearLab_to_RGB(lab,rgb);
                if (hdr) {
                    pixels[x].red = toHDR(rgb[0]);
                    pixels[x].green = toHDR(rgb[1]);
                    pixels[x].blue = toHDR(rgb[2]);
                }
                else {
                    pixels[x].red = DF_ROUND(rgb[0]);
                    pixels[x].green = DF_ROUND(rgb[1]);
                    pixels[x].blue = DF_ROUND(rgb[2]);
                }
            }
            else {
                pixels[x].red = src[x].red;
                pixels[x].green = src[x].green;
                pixels[x].blue = src[x].blue;
            }
        }
        pixel_cache->sync();
    });
}

