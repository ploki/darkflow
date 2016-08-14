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
#include "shapedynamicrange.h"
#include "photo.h"
#include <Magick++.h>
#include "cielab.h"
#include "hdr.h"
#include "console.h"

using Magick::Quantum;
/*
    lx=10**-5
    f(x)=(sin((((x/5)*pi)+pi/2))/2.-1./2.)*(-log((lx)**(1/2.4))/log(10))
    g(x)=10**f(log(x)/log(10))
    plot [x=lx:1] x**(1./2.4),g(x**(16./(12)))

    tanh:
    set logscale xy 2
    f(x)=((tanh(2*x/(log(65536)/log(10))+1)-1-(tanh(1)-1))*log(65536)/log(10)/2)
    g(x)=10**f(log(x)/log(10))
    plot [x=1/(2.**16):1] [1/(2.**8):1] x**(1./2.4),g(x**(16./10))**(1/2.4)

*/

//sin douce
// double lx=pow(10,-5);
// #define func_f(x) ( (sin((((x/5.L)*M_PI)+M_PI/2.L))/2.L-1.L/2.L)*(-log(pow(lx,1.L/2.4L))/log(10)) )
//sin dure
// #define func_f(x) ( (sin((((x/5.L)*M_PI))))*(-log(pow     (lx,1.L/2.4L))/log(10)) )
//tanh - gamma
// #define func_f(x) ((tanh(2*x/(log(pow(2,IL_W))/log(10))+1.L)-1.L-(tanh(1)-1.L))*log(pow(2,IL_H))/log(10)/2)

static const double IL_W = 16;
static const double IL_H = 12;
static const double k1 = .30103; //???
static const double k2 = log(pow(2,IL_W))/log(10);
static const double k3 = log(pow(2,IL_H))/log(10)/2;

static inline double func_f(double x, double val) {
    return ( tanh( (val*k1) + 2.*x/k2 + 1.L )
            - 1.L
            - (tanh(1+(val*k1))-1.L)) * k3;
}
static inline double func_g(double x, double val) {
    return pow(10,func_f(log(x)/log(10),val));
}

ShapeDynamicRange::ShapeDynamicRange(ShapeDynamicRange::Shape shape, qreal dynamicRange, qreal exposure, bool labDomain, QObject *parent) :
    LutBased(parent),
    m_shape(shape),
    m_dynamicRange(dynamicRange),
    m_exposure(exposure),
    m_labDomain(labDomain)
{
    //log2(log2(DR)) may sounds weird but it was this way in not-so-original
    //FIXME verify this

    dynamicRange = log2(m_dynamicRange);
    double val = log2(m_exposure)/pow(2.,log2(dynamicRange)-3.);
    bool stop=false;
    bool hdrStop=false;
    for ( int i = QuantumRange ; i >= 0 ; --i ) {
        double xx=double(i)/QuantumRange;
        double hx=fromHDR(i)/QuantumRange;
        m_lut[i]=func_g(pow(xx,16.L/dynamicRange), val)*QuantumRange;
        m_hdrLut[i]=toHDR(func_g(pow(hx,16.L/dynamicRange), val)*QuantumRange);

        //afin d'éviter que le sin ne remonte
        if ( stop )
            m_lut[i]=m_lut[i+1];
        else if ( i!=QuantumRange && m_lut[i] > m_lut[i+1] ) {
            stop=true;
            m_lut[i]=m_lut[i+1];
        }
        if ( hdrStop )
            m_hdrLut[i]=m_hdrLut[i+1];
        else if ( i!=QuantumRange && m_hdrLut[i] > m_hdrLut[i+1] ) {
            hdrStop=true;
            m_hdrLut[i]=m_hdrLut[i+1];
        }
    }
    //fait en sorte que le pied soit toujours <=  la droite y=x afin de conserver le point noir
    //valable pour plage dynamique <= 13 car au delà, la courbe est toujours sup
}

void ShapeDynamicRange::applyOnImage(Magick::Image& image, bool hdr)
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
        if ( error || ! pixels || !src ) {
            if (!error)
                dflError(DF_NULL_PIXELS);
            error=true;
            continue;
        }
        for (int x = 0 ; x < w ; ++x ) {
            quantum_t rgb[3];
            rgb[0] = src[x].red;
            rgb[1] = src[x].green;
            rgb[2] = src[x].blue;
            if (hdr) {
             if ( m_labDomain )    {
                 double cur = LUMINANCE(fromHDR(rgb[0]),
                                        fromHDR(rgb[1]),
                                        fromHDR(rgb[1]));
                 double lum = fromHDR(m_hdrLut[clamp(toHDR(cur))]);
                 double mul = log2(lum/cur)*4096;
                 rgb[0] = mul + rgb[0];
                 rgb[1] = mul + rgb[1];
                 rgb[2] = mul + rgb[2];
             }
             else {
                 rgb[0] = m_hdrLut[rgb[0]];
                 rgb[1] = m_hdrLut[rgb[1]];
                 rgb[2] = m_hdrLut[rgb[2]];
             }
            }
            else {
                if ( m_labDomain ) {
                    double cur = LUMINANCE(rgb[0],
                                           rgb[1],
                                           rgb[2]);
                    double lum = m_lut[clamp<quantum_t>(DF_ROUND(cur))];
                    double mul = lum/cur;
                    rgb[0] = mul*rgb[0];
                    rgb[1] = mul*rgb[1];
                    rgb[2] = mul*rgb[2];
                }
                else {
                    rgb[0] = m_lut[rgb[0]];
                    rgb[1] = m_lut[rgb[1]];
                    rgb[2] = m_lut[rgb[2]];
                }
            }
            pixels[x].red = clamp(rgb[0]);
            pixels[x].green = clamp(rgb[1]);
            pixels[x].blue = clamp(rgb[2]);

        }
        pixel_cache->sync();
    });
}
