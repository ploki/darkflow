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
#include "discretefouriertransform.h"
#include <QObject>
#include <Magick++.h>
#include "photo.h"
#include "algorithm.h"
#include "console.h"
#include "hdr.h"

using Magick::Quantum;

Q_STATIC_ASSERT( sizeof(fftw_complex) == sizeof(std::complex<double>));

__attribute__((constructor))
static void
init()
{
    fftw_init_threads();
}

DiscreteFourierTransform::DiscreteFourierTransform(Magick::Image &image, Photo::Gamma scale)
    : m_w(image.columns()),
      m_h(image.rows()),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    std::complex<double> *input = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    //std::complex<double> *output = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    Ordinary::Pixels cache(image);
    const Magick::PixelPacket *pixels = cache.getConst(0, 0, m_w, m_h);
    for (int c = 0 ; c < 3 ; ++c ) {
        for ( int y = 0 ; y < m_h ; ++y ) {
            for ( int x = 0 ; x < m_w ; ++x ) {
                quantum_t p = 0;
                double pixel;
                switch(c) {
                case 0: p = pixels[y*m_w+x].red; break;
                case 1: p = pixels[y*m_w+x].green; break;
                case 2: p = pixels[y*m_w+x].blue; break;
                }
                if ( Photo::HDR == scale )
                    pixel = fromHDR(p)/QuantumRange;
                else
                    pixel = double(p)/QuantumRange;
                input[y*m_w+x] = std::complex<double>(pixel, 0);
            }
        }
        std::complex<double> *plane = 0;
        switch(c) {
        case 0: plane = red; break;
        case 1: plane = green; break;
        case 2: plane = blue; break;
        }
        fftw_plan plan = fftw_plan_dft_2d(m_h, m_w,
                                          reinterpret_cast<fftw_complex*>(input),
                                          reinterpret_cast<fftw_complex*>(plane), FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
//        memcpy(plane, output, sizeof(fftw_complex)*m_h*m_w);
        fftw_destroy_plan(plan);
    }
    //fftw_free(output);
    fftw_free(input);
}

DiscreteFourierTransform::DiscreteFourierTransform(Magick::Image &magnitude,
                                                   Magick::Image &phase,
                                                   Photo::Gamma scale,
                                                   double normalization)
    : m_w(magnitude.columns()),
      m_h(magnitude.rows()),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    int p_w = phase.columns();
    int p_h = phase.rows();
    Ordinary::Pixels mCache(magnitude);
    Ordinary::Pixels pCache(phase);
    const Magick::PixelPacket *mPixels = mCache.getConst(0, 0, m_w, m_h);
    const Magick::PixelPacket *pPixels = pCache.getConst(0, 0, p_w, p_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            for ( int c = 0 ; c < 3 ; ++c ) {
                quantum_t q_mag = 0;
                quantum_t q_pha = 0;
                int xx = (m_w+x-m_w/2)%m_w;
                int yy = (m_h+y-m_h/2)%m_h;
                int px = xx%p_w;
                int py = yy%p_h;
                std::complex<double> *plane = 0;
                switch (c) {
                case 0: plane = red; q_mag = mPixels[yy*m_w+xx].red; q_pha = pPixels[py*p_w+px].red; break;
                case 1: plane = green; q_mag = mPixels[yy*m_w+xx].green; q_pha = pPixels[py*p_w+px].green; break;
                case 2: plane = blue; q_mag = mPixels[yy*m_w+xx].blue; q_pha = pPixels[py*p_w+px].blue; break;
                }
                double r_mag = normalization * (Photo::HDR == scale ? fromHDR(q_mag) : q_mag);
                double r_pha = (2.*M_PI*double(q_pha)/QuantumRange)-M_PI;
                plane[y*m_w+x] = std::polar(r_mag, r_pha);
            }
        }
    }
}

DiscreteFourierTransform::~DiscreteFourierTransform()
{
    fftw_free(red);
    fftw_free(green);
    fftw_free(blue);
}

Magick::Image DiscreteFourierTransform::reverse(double luminosity)
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    std::complex<double> *input = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    std::complex<double> *output = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    for ( int c = 0 ; c < 3 ; ++c ) {
        std::complex<double> *plane = 0;
        switch(c) {
        case 0: plane = red; break;
        case 1: plane = green; break;
        case 2: plane = blue; break;
        }
        memcpy(input, plane, sizeof(fftw_complex)*m_h*m_w);
        fftw_plan plan = fftw_plan_dft_2d(m_h, m_w,
                                          reinterpret_cast<fftw_complex*>(input),
                                          reinterpret_cast<fftw_complex*>(output), FFTW_BACKWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
        for ( int y = 0 ; y < m_h ; ++y ) {
            for ( int x = 0 ; x < m_w ; ++x ) {
                quantum_t pixel = clamp(luminosity*output[y*m_w+x].real()*QuantumRange/(m_w*m_h));
                switch(c) {
                case 0: pixels[y*m_w+x].red = pixel; break;
                case 1: pixels[y*m_w+x].green = pixel; break;
                case 2: pixels[y*m_w+x].blue = pixel; break;
                }
            }
        }
        fftw_destroy_plan(plan);
    }
    cache.sync();
    fftw_free(output);
    fftw_free(input);
    return image;
}

Magick::Image DiscreteFourierTransform::imageMagnitude(Photo::Gamma scale, double *normalizationp)
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    double max = 0;
    for (int i = 0, s = m_w*m_h ; i < s ; ++i) {
        max = qMax(max, qMax(std::abs(red[i]), qMax(std::abs(green[i]), std::abs(blue[i]))));
    }
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            int xx = (x+m_w/2)%m_w;
            int yy = (y+m_h/2)%m_h;
            double r = std::abs(red[y*m_w+x]) * QuantumRange / max,
                   g = std::abs(green[y*m_w+x]) * QuantumRange / max,
                   b = std::abs(blue[y*m_w+x]) * QuantumRange / max;
            if ( scale == Photo::HDR ) {
                pixels[yy*m_w+xx].red = toHDR(r);
                pixels[yy*m_w+xx].green = toHDR(g);
                pixels[yy*m_w+xx].blue = toHDR(b);
            }
            else {
                pixels[yy*m_w+xx].red = clamp<quantum_t>(r);
                pixels[yy*m_w+xx].green = clamp<quantum_t>(g);
                pixels[yy*m_w+xx].blue = clamp<quantum_t>(b);
            }
        }
    }
    cache.sync();
    if (normalizationp)
        *normalizationp = max / QuantumRange;
    return image;
}

Magick::Image DiscreteFourierTransform::imagePhase()
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            int xx = (x+m_w/2)%m_w;
            int yy = (y+m_h/2)%m_h;
            pixels[yy*m_w+xx].red = clamp<quantum_t>( (std::arg(red[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
            pixels[yy*m_w+xx].green = clamp<quantum_t>( (std::arg(green[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
            pixels[yy*m_w+xx].blue = clamp<quantum_t>( (std::arg(blue[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
        }
    }
    cache.sync();
    return image;
}

DiscreteFourierTransform &DiscreteFourierTransform::operator/=(const DiscreteFourierTransform &other)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] /=  ( other.red[i] == 0. ? 1e-12 : other.red[i]);
        green[i] /= ( other.green[i] == 0. ? 1e-12 : other.green[i]);
        blue[i] /= ( other.blue[i] == 0. ? 1e-12 : other.blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::operator*=(const DiscreteFourierTransform &other)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] *= other.red[i];
        green[i] *= other.green[i];
        blue[i] *= other.blue[i];
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::conj()
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::conj(red[i]);
        green[i] = std::conj(green[i]);
        blue[i] = std::conj(blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::inv()
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = 1. / ( red[i] == 0. ? 1e-12 : red[i]);
        green[i] = 1. / ( green[i] == 0. ? 1e-12 : green[i]);
        blue[i] = 1. / ( blue[i] == 0. ? 1e-12 : blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::wienerFilter(double k)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::conj(red[i])/(pow(std::abs(red[i]),2)+k);
        green[i] = std::conj(green[i])/(pow(std::abs(green[i]),2)+k);
        blue[i] = std::conj(blue[i])/(pow(std::abs(blue[i]),2)+k);
    }
    return *this;
}

DiscreteFourierTransform::DiscreteFourierTransform(const DiscreteFourierTransform &other)
    : m_w(other.m_w),
      m_h(other.m_h),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    memcpy(red, other.red, sizeof(fftw_complex)*m_h*m_w);
    memcpy(green, other.green, sizeof(fftw_complex)*m_h*m_w);
    memcpy(blue, other.blue, sizeof(fftw_complex)*m_h*m_w);
}

Magick::Image DiscreteFourierTransform::normalize(Magick::Image &image, int w, bool center)
{
    int h = w;
    int k_w = image.columns();
    int k_h = image.rows();
    Magick::Image nk(Magick::Geometry(w, h), Magick::Color(0,0,0));
    int o_x = (w-k_w)/2;
    int o_y = (h-k_h)/2;
    std::shared_ptr<Ordinary::Pixels> i_cache(new Ordinary::Pixels(image));
    std::shared_ptr<Ordinary::Pixels> n_cache(new Ordinary::Pixels(nk));
    dfl_parallel_for(y, 0, k_h, 4, (image, nk), {
        const Magick::PixelPacket * k_pixel = i_cache->getConst(0, y, k_w, 1);
        Magick::PixelPacket * n_pixel;
        if (center)
            n_pixel = n_cache->get(o_x, o_y+y, k_w, 1);
        else
            n_pixel = n_cache->get(0, y, k_w, 1);
        for ( int x = 0 ; x < k_w ; ++x ) {
            n_pixel[x] = k_pixel[x];
        }
        n_cache->sync();
    });
    return nk;
}

Magick::Image DiscreteFourierTransform::roll(Magick::Image &image, int o_x, int o_y)
{
    int w = image.columns();
    int h = image.rows();
    Magick::Image nk(Magick::Geometry(w, h), Magick::Color(0,0,0));
    std::shared_ptr<Ordinary::Pixels> i_cache(new Ordinary::Pixels(image));
    std::shared_ptr<Ordinary::Pixels> n_cache(new Ordinary::Pixels(nk));
    dfl_parallel_for(y, 0, h, 4, (image, nk), {
        const Magick::PixelPacket * k_pixel = i_cache->getConst(0, y, w, 1);
        Magick::PixelPacket * n_pixel= n_cache->get(0, (y+o_y+h)%h, w, 1);
        for ( int x = 0 ; x < w ; ++x ) {
            n_pixel[(x+o_x+w)%w] = k_pixel[x];
        }
        n_cache->sync();
    });
    return nk;
}
