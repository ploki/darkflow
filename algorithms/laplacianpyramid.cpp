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
#include "laplacianpyramid.h"

LaplacianPyramid::LaplacianPyramid(const GaussianPyramid &gaussianPyramid) :
    Pyramid(gaussianPyramid.base(), gaussianPyramid.base())
{
    int J = levels();
    const pFloat *src;
    pFloat *dst;
    int b;

    // initialize plan J-1
    src = gaussianPyramid.getPlane(J-1);
    dst = getPlane(J-1);
    b = planeBase(J-1);
    for (int i = 0 ; i < b * b ; ++i) {
        dst[i] = src[i];
    }

    for (int j = J - 2 ; j >= 0 ; --j) {
        b = planeBase(j);
        src = gaussianPyramid.getPlane(j);
        dst = getPlane(j);
        gaussianPyramid.upsample(j+1, dst);
        for (int y = 0 ; y < b ; ++y) {
            for (int x = 0 ; x < b ; ++x) {
                pFloat tmp = src[y*b+x];
                tmp -= dst[y*b+x];
                dst[y*b+x] = tmp;
            }
        }
    }
}

LaplacianPyramid::LaplacianPyramid(int columns, int rows) :
    Pyramid(columns, rows)
{
}

void LaplacianPyramid::collapse(GaussianPyramid &gp)
{
    Q_ASSERT(gp.levels() == levels());
    Q_ASSERT(gp.base() == base());
    Pyramid::pFloat *tmp(new Pyramid::pFloat[base()*base()]);
    int J = levels();
    Pyramid::pFloat *gn = gp.getPlane(J-1);
    Pyramid::pFloat *ln = getPlane(J-1);
    for (int i = 0 ; i < planeBase(J-1)*planeBase(J-1) ; ++i) {
        ln[i] = gn[i];
    }

    for (int l = J-2 ; l >= 0 ; --l) {
        upsample(l+1, tmp);
        ln = getPlane(l);
        int b = planeBase(l);
        for (int y = 0 ; y < b ; ++y) {
            for (int x = 0 ; x < b ; ++x) {
                ln[y*b+x] += tmp[y*b+x];
            }
        }
    }
    delete[] tmp;
}
