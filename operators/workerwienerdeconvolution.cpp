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
#include "opwienerdeconvolution.h"
#include "workerwienerdeconvolution.h"
#include <list>
#include "algorithm.h"
#include "discretefouriertransform.h"

using Magick::Quantum;

WorkerWienerDeconvolution::WorkerWienerDeconvolution(qreal luminosity, qreal snr, int iterations, QThread *thread, OpWienerDeconvolution *op) :
    OperatorWorker(thread, op),
    m_luminosity(luminosity),
    m_snr(snr),
    m_iterations(iterations)
{
}

Photo WorkerWienerDeconvolution::process(const Photo &photo, int, int)
{
    return photo;
}


static inline Magick::Image
normalizeImage(Magick::Image& image, int w, int h, bool center)
{
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

static inline Magick::Image roll(Magick::Image& image, int o_x, int o_y)
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

void WorkerWienerDeconvolution::deconv(Magick::Image& image, Magick::Image& kernel, qreal luminosity)
{
#ifdef USING_GRAPHICSMAGICK
    Q_UNUSED(image);
    Q_UNUSED(kernel);
    Q_UNUSED(luminosity);
    dflCritical(tr("Fourier Transformation not available with GraphicsMagick"));
    return;
#else
    Magick::Image nk = normalizeImage(kernel, qMax(image.columns(),image.rows()), qMax(image.columns(),image.rows()), true);
    Magick::Image ni = normalizeImage(image, qMax(image.columns(),image.rows()), qMax(image.columns(),image.rows()), false);

    Magick::Image nnk = roll(nk,-int(nk.columns())/2, -int(nk.rows())/2);

    DiscreteFourierTransform fft_image(ni);
    DiscreteFourierTransform fft_kernel(nnk);

    fft_kernel.wienerFilter(1./m_snr);
    for (int i = 0 ; i < m_iterations ; ++i)
        fft_image *= fft_kernel;

#define RM(comp) \
    Rm_pxl[x].comp = clamp( luminosity*double(Bm_pxl[x].comp)*(double(Am_pxl[x].comp)/(double(Am_pxl[x].comp)*Am_pxl[x].comp+1./m_snr)))
#define mod(a,b) (a)%(b)
#define RP(comp) \
    Rp_pxl[x].comp = mod( quantum_t(Bp_pxl[x].comp) - quantum_t(Ap_pxl[x].comp) + 65535 + 32768, 65536)
    image = fft_image.reverse(luminosity);
#endif
}

void WorkerWienerDeconvolution::play()
{
    Q_ASSERT( m_inputs.count() == 2 );

    if ( m_inputs[1].count() == 0 )
        return OperatorWorker::play();

    int k_count = m_inputs[1].count();
    int complete = qMin(1,k_count) * m_inputs[0].count();
    int n = 0;
    foreach(Photo photo, m_inputs[0]) {
        if (aborted())
            continue;
        try {
            Magick::Image& image = photo.image();
            Magick::Image& kernel = m_inputs[1][n%k_count].image();
            int w=image.columns();
            int h=image.rows();
            deconv(image, kernel, m_luminosity);
            image.page(Magick::Geometry(0,0,0,0));
            image.crop(Magick::Geometry(w, h));
            outputPush(0, photo);
            emitProgress(n,complete, 1, 1);
            ++n;
        }
        catch (std::exception &e) {
            setError(photo, e.what());
            setError(m_inputs[1][n%k_count], e.what());
        }
    }
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
}
