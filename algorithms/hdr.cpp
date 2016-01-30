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
#include "hdr.h"
#include <Magick++.h>

using Magick::Quantum;

double *fromHDRLut;

class onStart {
public:
    onStart() {
        fromHDRLut = new double[65536];
        for ( int v = 0 ; v <= 65535 ; ++v ) {
#if 1
            if ( v == 0 ) fromHDRLut[v]=0;
            else fromHDRLut[v] = pow(2,(double)(v+1)/4096)-1;
#else
            if ( v == 0 ) return 0;
            return pow(2,(double)(v)/4096);
#endif
        }
    }
    ~onStart() {
        delete[] fromHDRLut;
    }
} once;


HDR::HDR(bool revert, QObject *parent) :
    LutBased(parent),
    m_revert(revert)
{

#pragma omp parallel for dfl_threads(1024)
    for(int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        m_hdrLut[i] = clamp( revert
                          ? DF_ROUND(fromHDR(i))
                          : i);
    }
#pragma omp parallel for dfl_threads(1024)
    for(int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        m_lut[i] = clamp( revert
                          ? i
                          : toHDR(i));
    }
}

void HDR::applyOn(Photo &photo)
{
    bool hdr = photo.getScale() == Photo::HDR;
    applyOnImage(photo.image(), hdr);
    if (m_alterCurve)
        applyOnImage(photo.curve(), hdr);
    photo.setTag(TAG_SCALE,
                 m_revert
                 ? TAG_SCALE_LINEAR
                 : TAG_SCALE_HDR );
}
