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
#include "whitebalance.h"
#include "photo.h"
#include "console.h"
#include <Magick++.h>
#include <cmath>

WhiteBalance::WhiteBalance(qreal temperature,
                           qreal tint,
                           bool safe,
                           QObject *parent) :
    Algorithm(true, parent),
    m_temperature(temperature),
    m_tint(tint),
    m_safe(safe),
    m_rgb()
{
    m_temperature = clamp<double>(m_temperature,2000,12000);
    Temperature_to_RGB(temperature,m_rgb);

    m_rgb[1] = m_rgb[1] / m_tint;

    m_rgb[0]=1/m_rgb[0];
    m_rgb[1]=1/m_rgb[1];
    m_rgb[2]=1/m_rgb[2];

    double minmul=m_rgb[0];
    if (minmul > m_rgb[1] ) minmul=m_rgb[1];
    if (minmul > m_rgb[2] ) minmul=m_rgb[2];
    m_rgb[0]/=minmul;
    m_rgb[1]/=minmul;
    m_rgb[2]/=minmul;

    if ( m_safe ) {
        double maxmul=1;
        if ( maxmul < m_rgb[0] ) maxmul=m_rgb[0];
        if ( maxmul < m_rgb[1] ) maxmul=m_rgb[1];
        if ( maxmul < m_rgb[2] ) maxmul=m_rgb[2];
        m_rgb[0]/=maxmul;
        m_rgb[1]/=maxmul;
        m_rgb[2]/=maxmul;
    }
}

void WhiteBalance::applyOnImage(Magick::Image& image, bool hdr)
{
    Magick::Image srcImage(image);
    ResetImage(image);
    int h = image.rows(),
            w = image.columns();
    std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
    std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
    dfl_block_array(double, rgb, 3);
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
    dfl_parallel_for(y, 0, h, 4, (srcImage, image), {
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

void WhiteBalance::Temperature_to_RGB(qreal T, qreal RGB[]) {
    //thanks to ufraw_routines.c from ufraw-0.11
    // and http://www.brucelindbloom.com/index.html?Eqn_T_to_xy.html

    const double XYZ_to_RGB[3][3] = {
        { 3.24071,  -0.969258,  0.0556352 },
        {-1.53726,  1.87599,    -0.203996 },
        {-0.498571, 0.0415557,  1.05707 } };

    int c;
    double xD, yD, X, Y, Z, max;
    // Fit for CIE Daylight illuminant
    if (T<= 4000) {
        xD = 0.27475e9/(T*T*T) - 0.98598e6/(T*T) + 1.17444e3/T + 0.145986;
    } else if (T<= 7000) {
        xD = -4.6070e9/(T*T*T) + 2.9678e6/(T*T) + 0.09911e3/T + 0.244063;
    } else {
        xD = -2.0064e9/(T*T*T) + 1.9018e6/(T*T) + 0.24748e3/T + 0.237040;
    }
    yD = -3*xD*xD + 2.87*xD - 0.275;

    // Fit for Blackbody using CIE standard observer function at 2 degrees
    //xD = -1.8596e9/(T*T*T) + 1.37686e6/(T*T) + 0.360496e3/T + 0.232632;
    //yD = -2.6046*xD*xD + 2.6106*xD - 0.239156;

    // Fit for Blackbody using CIE standard observer function at 10 degrees
    //xD = -1.98883e9/(T*T*T) + 1.45155e6/(T*T) + 0.364774e3/T + 0.231136;
    //yD = -2.35563*xD*xD + 2.39688*xD - 0.196035;

    X = xD/yD;
    Y = 1;
    Z = (1-xD-yD)/yD;
    max = 0;
    for (c=0; c<3; c++) {
        RGB[c] = X*XYZ_to_RGB[0][c] + Y*XYZ_to_RGB[1][c] + Z*XYZ_to_RGB[2][c];
        if (RGB[c]>max) max = RGB[c];
    }
    for (c=0; c<3; c++) RGB[c] = RGB[c]/max;
}
