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
#ifndef DISCRETEFOURIERTRANSFORM_H
#define DISCRETEFOURIERTRANSFORM_H

#include <complex>
#include <fftw3.h>
#include "photo.h"

namespace Magick {
class Image;
}
class DiscreteFourierTransform
{
    int m_w;
    int m_h;
    std::complex<double> *red;
    std::complex<double> *green;
    std::complex<double> *blue;
public:
    DiscreteFourierTransform(Magick::Image& image, Photo::Gamma scale);
    DiscreteFourierTransform(Magick::Image& magnitude, Magick::Image& phase, Photo::Gamma scale, double normalization);
    ~DiscreteFourierTransform();
    Magick::Image reverse(double luminosity);
    Magick::Image imageMagnitude(Photo::Gamma scale, double *normalizationp);
    Magick::Image imagePhase();

    DiscreteFourierTransform& operator/=(const DiscreteFourierTransform& other);
    DiscreteFourierTransform& operator*=(const DiscreteFourierTransform& other);

    DiscreteFourierTransform& conj();
    DiscreteFourierTransform& inv();
    DiscreteFourierTransform& wienerFilter(double k);

    DiscreteFourierTransform(const DiscreteFourierTransform &other);


    static Magick::Image normalize(Magick::Image& image, int w, bool center);
    static Magick::Image roll(Magick::Image& image, int o_x, int o_y);
};

#endif // DISCRETEFOURIERTRANSFORM_H
