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
#include <QtGlobal>
#include "pyramid.h"
#include "algorithm.h"

static int
calculateLevels(int base)
{
    int levels = 1;
    while (base > 1) {
        ++levels;
        base /= 2;
    }
    return levels;
}
static int
calculateArraySize(int base) {
    return 4*base*base/3;
}

static long
offsetOfLevel(int base, int level) {
    long offset = 0;
    for (int i = 0 ; i < level ; ++i) {
        offset += base * base;
        base /= 2;
    }
    return offset;
}

static int
nextPowerOfTwo(int n) {
    int p = 1;
    while (p < n) {
        p<<=1;
    }
    return p;
}

Pyramid::Pyramid(int columns, int rows) :
    m_base(nextPowerOfTwo(qMax(rows, columns))),
    m_originRows(rows),
    m_originColumns(columns),
    m_levels(calculateLevels(m_base)),
    m_pyramid(new pFloat[calculateArraySize(m_base)])
{
}

Pyramid::~Pyramid()
{
    delete[] m_pyramid;
}

int Pyramid::levels() const
{
    return m_levels;
}

int Pyramid::base() const
{
    return m_base;
}

int Pyramid::originColumns() const
{
    return m_originColumns;
}

int Pyramid::originRows() const
{
    return m_originRows;
}

int Pyramid::planeBase(int level) const
{
    if (level > m_levels)
        return 0;
    return m_base / (1 << level);
}

Pyramid::pFloat *Pyramid::getPlane(int level)
{
    if (level >= levels())
        return nullptr;
    long offset = offsetOfLevel(m_base, level);
    return m_pyramid + offset;
}

const Pyramid::pFloat *Pyramid::getPlane(int level) const
{
    if (level >= levels())
        return nullptr;
    return m_pyramid + offsetOfLevel(m_base, level);
}

//downsample the level into the specified buffer, maybe the next level plane
void Pyramid::downsample(int level, pFloat *dstPlane) const
{
    const int order = sizeof(downsampleKernel)/sizeof(*downsampleKernel);
    double kernel[order*order];
    kernel_1D_to_2D(downsampleKernel, kernel, order);
    const pFloat *srcPlane = getPlane(level);
    int srcBase = planeBase(level);
    int dstBase = srcBase/2;
    for (int y = 0 ; y < dstBase ; ++y) {
        for (int x = 0 ; x < dstBase ; ++x) {
            dstPlane[y*dstBase+x] = 0.;
            for (int i = 0 ; i < order ; ++i) {
                int yy = clamp(y*2+i-1, 0, srcBase-1);
                for (int j = 0 ; j < order ; ++j) {
                    int xx = clamp(x*2+j-1, 0, srcBase-1);
                    pFloat tmp = srcPlane[yy*srcBase+xx];
                    tmp *= kernel[i*order+j];
                    dstPlane[y*dstBase+x] += tmp;
                }
            }
        }
    }
}

// dstPlane MUST be of level-1 size
void Pyramid::upsample(int level, pFloat *dstPlane) const
{
    const int order = sizeof(upsampleKernel)/sizeof(*upsampleKernel);
    double kernel[order*order];
    kernel_1D_to_2D(upsampleKernel, kernel, order);
    const pFloat *srcPlane = getPlane(level);
    int srcBase = planeBase(level);
    int dstBase = planeBase(level - 1);
    for (int y = 0 ; y < dstBase; ++y) {
        for (int x = 0 ; x < dstBase ; ++x) {
            dstPlane[y*dstBase+x] = 0;
            int cx[] = { (x/2) - 1 + 2*(x % 2), x/2 };
            int cy[] = { (y/2) - 1 + 2*(y % 2), y/2 };
            for (int i = 0 ; i < 2 ; ++i) {
                int yy = clamp(cy[i],0, srcBase-1);
                for (int j = 0 ; j < 2 ; ++j) {
                    int xx = clamp(cx[j],0, srcBase-1);
                    pFloat tmp = srcPlane[yy*srcBase+xx];
                    tmp *= kernel[i*order+j];
                    dstPlane[y*dstBase+x] += tmp;
                }
            }
        }
    }
}

int Pyramid::baseForLevels(int levels)
{
    return 1<<(levels-1);
}
