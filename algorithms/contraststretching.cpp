/*
 * Copyright (c) 2006-2021, Guillaume Gimenez <guillaume@blackmilk.fr>
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

#include "contraststretching.h"
#include "hdr.h"

using Magick::Quantum;

static qreal lin2EV(qreal x)
{
    //domain : 1/QuantumRange - 1, linear
    // -log2(QuantumRange) - 0 -> -15.9999779861IL - 0IL
    //codomain : 0 - 1, log, 1IL step = 1/16
    return log(x) / log(QuantumRange + 1) + 1;
}

static qreal EV2lin(qreal y) {
    //y=log(x)/log(Q+1)+1
    //y-1=log(x)/log(Q+1)
    //(y-1)*log(Q+1) = log(x)
    //x = (Q+1)^(y-1)
    return pow(QuantumRange + 1, y - 1);
}

/*
    g(x) = x^g
    g'(x)= 1/x g x^g
    h(x) = g(x/xl)*yl
 -> h(x) = x^g xl^(-g) yl
    h'(x) = yl/xl * g'(x/xl)
    h'(x) = yl/xl * xl/x g(x/xl)^{g}
    h'(xl) = slope = yl/xl * xl/xl g(xl/xl)^{g}
         slope = yl/xl * g * 1^{g}
         slope = yl/xl * g
 ->     g = slope*xl/yl
*/
qreal ContrastStretching::heel(qreal x)
{
    //return pow(x/m_xl, m_g_heel) * m_yl;
    return m_bp + pow(x/m_xl, m_g_heel) * (m_yl-m_bp);

}

qreal ContrastStretching::shoulder(qreal x)
{
    return 1. - (pow((1.-x)/(1.-m_xh), m_g_shoulder) * (1.-m_yh));
}

qreal ContrastStretching::curve(qreal x)
{
    if ( x < m_xl) {
        return heel(x);
    } else if ( x <= m_xh) {
        return m_slope * x + m_y0;
    } else {
        return shoulder(x);

    }
}

ContrastStretching::ContrastStretching(qreal bp,
                                       qreal xl, qreal yl,
                                       qreal xh, qreal yh,
                                       QObject *parent) :
    LutBased(parent),
    m_bp(lin2EV(bp)),
    m_xl(lin2EV(xl)),
    m_yl(lin2EV(yl)),
    m_xh(lin2EV(xh)),
    m_yh(lin2EV(yh)),
    m_slope((m_yh - m_yl) / (m_xh - m_xl)),
    m_g_heel(m_slope*m_xl/(m_yl-m_bp)),
    m_g_shoulder(m_slope*(1.-m_xh)/(1.-m_yh)),
    m_y0(m_yh - m_slope * m_xh)
{
    if (m_xh == m_xl) {
        //XXX
        m_y0 = m_g_heel = m_g_shoulder = m_slope = 0;
        qDebug("plop!");
    }
    qDebug("m_xl=%f, m_xh=%f", m_xl, m_xh);
    dfl_parallel_for(i, 0, int(QuantumRange+1), 1024, (), {
        m_lut[i] = EV2lin(curve(lin2EV(qreal(i)/qreal(QuantumRange))))*QuantumRange;
        m_hdrLut[i] = toHDR(EV2lin(curve(lin2EV(fromHDR(i)/qreal(QuantumRange))))*QuantumRange);
    });
}

