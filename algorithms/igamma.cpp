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
#include <Magick++.h>
#include "igamma.h"
#include "photo.h"
#include "hdr.h"

using Magick::Quantum;

iGamma::iGamma(qreal gamma, qreal x0, bool invert, QObject *parent) :
    LutBased(parent),
    m_gamma(gamma),
    m_x0(x0),
    m_invert(invert)
{
    double a = - ( gamma - 1.L)*pow(x0,1.L/gamma)/((gamma-1.L)*pow(x0,1.L/gamma)-gamma);
    double p=0;
    if ( invert ) {
        //recalcul de x0 et a
        x0=(1.L+a)*pow(x0,1.L/gamma)-a;
        a=(gamma-1.L)*x0;
        p=gamma*pow((x0+a)/(a+1.L),gamma)/(x0+a);
    }
    else
        p=(a+1.L)*pow(x0,1.L/gamma)/(gamma*x0);

#pragma omp parallel for dfl_threads(1024)
    for ( int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        double xx= double(i)/double(QuantumRange);
        if ( xx > x0 ) {
            if ( invert ) {
                m_lut[i]=pow(((xx+a)/(a+1.L)),gamma)*QuantumRange;
            }
            else {
                m_lut[i]= ((1.L+a)*pow(xx,(1.L/gamma))-a)*QuantumRange;
            }
        }
        else {
            m_lut[i]=p*xx*double(QuantumRange);
        }
    }
// reverse and hdr are not compatible
#pragma omp parallel for dfl_threads(1024)
    for ( int i = 0 ; i <= int(QuantumRange) ; ++i ) {

        double xx= double(fromHDR(i))/double(QuantumRange);
        if ( xx > x0 ) {
            if ( invert ) {
                m_hdrLut[i]=pow(((xx+a)/(a+1.L)),gamma)*QuantumRange;
            }
            else {
                m_hdrLut[i]= ((1.L+a)*pow(xx,(1.L/gamma))-a)*QuantumRange;
            }
        }
        else {
            m_hdrLut[i]=p*xx*double(QuantumRange);
        }
    }


}

iGamma& iGamma::sRGB()
{
    static iGamma g(SRGB_G, SRGB_N,false);
    return g;
}

iGamma& iGamma::BT709()
{
    static iGamma g(2.222L, 0.018L,false);
    return g;
}

iGamma& iGamma::Lab()
{
    static iGamma g(3.0L, 0.008856L, false);
    return g;
}

iGamma& iGamma::reverse_sRGB()
{
    static iGamma g(SRGB_G, SRGB_N,true);
    return g;
}

iGamma& iGamma::reverse_BT709()
{
    static iGamma g(2.222L, 0.018L,true);
    return g;
}

iGamma& iGamma::reverse_Lab()
{
    static iGamma g(3.0L, 0.008856L, true);
    return g;
}

void iGamma::applyOn(Photo &photo)
{
    bool hdr = photo.getScale() == Photo::HDR;
    applyOnImage(photo.image(), hdr);
    if (m_alterCurve)
        applyOnImage(photo.curve(), hdr);
    photo.setTag(TAG_SCALE,
                 m_invert
                 ? TAG_SCALE_LINEAR
                 : TAG_SCALE_NONLINEAR );
}

