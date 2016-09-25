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
#include "opconvolution.h"
#include "workerconvolution.h"
#include <list>
#include "algorithm.h"
#include "discretefouriertransform.h"

using Magick::Quantum;

WorkerConvolution::WorkerConvolution(qreal luminosity, QThread *thread, OpConvolution *op) :
    OperatorWorker(thread, op),
    m_luminosity(luminosity)
{
}

Photo WorkerConvolution::process(const Photo &photo, int, int)
{
    return photo;
}

void WorkerConvolution::conv(Magick::Image& image, Photo::Gamma imageScale,
                             Magick::Image& kernel, Photo::Gamma kernelScale,
                             qreal luminosity)
{
    Magick::Image nk = DiscreteFourierTransform::normalize(kernel, qMax(image.columns(),image.rows()), true);
    Magick::Image ni = DiscreteFourierTransform::normalize(image, qMax(image.columns(),image.rows()), false);

    Magick::Image nnk = DiscreteFourierTransform::roll(nk,-int(nk.columns())/2, -int(nk.rows())/2);

    DiscreteFourierTransform fft_image(ni, imageScale);
    DiscreteFourierTransform fft_kernel(nnk, kernelScale);

    fft_image *= fft_kernel;
    image = fft_image.reverse(luminosity);
}

void WorkerConvolution::play()
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
            conv(image, photo.getScale(), kernel, m_inputs[1][n%k_count].getScale(), m_luminosity);
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
