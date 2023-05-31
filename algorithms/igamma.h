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
#ifndef IGAMMA_H
#define IGAMMA_H

#include "lutbased.h"

#define SRGB_G 2.4L
#define SRGB_N 0.00304L


class iGamma : public LutBased
{
    Q_OBJECT
public:
    explicit iGamma(qreal gamma, qreal x0, bool invert = false, bool isHDR = false, QObject *parent = 0);
    static iGamma& sRGB();
    static iGamma& BT709();
    static iGamma& Lab();
    static iGamma& reverse_sRGB();
    static iGamma& reverse_BT709();
    static iGamma& reverse_Lab();

    void applyOn(Photo &photo);

private:
    qreal m_gamma;
    qreal m_x0;
    bool m_invert;
    bool m_isHDR;
};

#endif // IGAMMA_H
