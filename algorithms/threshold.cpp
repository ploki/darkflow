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
#include "threshold.h"
#include "hdr.h"

using Magick::Quantum;

Threshold::Threshold(qreal high, qreal low, QObject *parent) :
    LutBased(parent)
{
    quantum_t h = high * QuantumRange;
    quantum_t l = low * QuantumRange;

    quantum_t up = QuantumRange;
    quantum_t down = 0;
    if ( low > high ) {
        down = QuantumRange;
        up = 0;
        quantum_t tmp = h;
        h = l;
        l = tmp;
    }
    dfl_parallel_for(i, 0, int(QuantumRange+1), 1024, (), {
        if ( i >= l && i <= h )
            m_lut[i] = up;
        else
            m_lut[i] = down;
    });

    l=toHDR(l);
    h=toHDR(h);
    dfl_parallel_for(i, 0, int(QuantumRange+1), 1024, (), {
        if ( i >= l && i <= h )
            m_hdrLut[i] = up;
        else
            m_hdrLut[i] = down;
    });


}
