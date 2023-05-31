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
#include "opmicrocontrasts.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>
#include "cielab.h"
#include "hdr.h"

static void
normalizeKernel(int order, double *kernel)
{
  double div = 0;
  int i;
  int n = order * order;
  for ( i = 0 ; i < n ; ++i )
    div+=kernel[i];
  for ( i = 0 ; i < n ; ++i )
    kernel[i]/=div;
}

class WorkerMicroContrasts : public OperatorWorker {
public:
    WorkerMicroContrasts(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        double kernel[]={ 0 , -1.,  0,
                         -1.,  8., -1.,
                          0 , -1.,  0};
        normalizeKernel(3, kernel);
        Magick::Image& srcImage(const_cast<Magick::Image&>(photo.image()));
        Magick::Image& image(newPhoto.image());
        int w = image.columns(),
            h = image.rows();
        ResetImage(image);
        std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
        std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(image));
        Photo::Gamma pixelEncoding = photo.getScale();
        const qreal gammaCorrection = pow(QuantumRange, -1.2);
        const qreal invGammaCorrection = pow(QuantumRange, 1.2/2.2);
        dfl_parallel_for(y, 0, h, 4, (srcImage), {
                             const Magick::PixelPacket *srcPixel[3] =
                             { y > 0 ? src_cache->getConst(0, y-1, w, 1) : nullptr,
                               src_cache->getConst(0, y, w, 1),
                               y < h - 1 ? src_cache->getConst(0, y+1, w, 1) : nullptr };
                             Magick::PixelPacket *pixel = cache->get(0, y, w, 1);
                             for (int x = 0 ; x < w; ++x) {
                                if ( y == 0 || y == h - 1 || x == 0 || x == w - 1) {
                                    pixel[x] = srcPixel[1][x];
                                 } else {
                                     qreal currgb[3] = {};
                                     //apply kernel to src pixel and its neighborhood on lum
                                     qreal p = 0;
                                     for (int j = 0; j < 3; ++j) {
                                         for (int i = 0; i < 3; ++i) {
                                             qreal linrgb[3] = {};
                                             if (pixelEncoding == Photo::HDR) {
                                                 linrgb[0] = fromHDR(srcPixel[j][x+i-1].red);
                                                 linrgb[1] = fromHDR(srcPixel[j][x+i-1].green);
                                                 linrgb[2] = fromHDR(srcPixel[j][x+i-1].blue);
                                             } else if (pixelEncoding == Photo::Linear) {
                                                 linrgb[0] = srcPixel[j][x+i-1].red;
                                                 linrgb[1] = srcPixel[j][x+i-1].green;
                                                 linrgb[2] = srcPixel[j][x+i-1].blue;
                                             } else {
                                                 linrgb[0] = gammaCorrection * pow(srcPixel[j][x+i-1].red, 2.2);
                                                 linrgb[1] = gammaCorrection * pow(srcPixel[j][x+i-1].green, 2.2);
                                                 linrgb[2] = gammaCorrection * pow(srcPixel[j][x+i-1].blue, 2.2);
                                             }
                                             p += kernel[j*3+i] * LUMINANCE(linrgb[0], linrgb[1], linrgb[2]);
                                             if (i == 1 && j == 1) {
                                                 currgb[0] = linrgb[0];
                                                 currgb[1] = linrgb[1];
                                                 currgb[2] = linrgb[2];
                                             }
                                         }
                                     }
                                     //merge computed lum with srcpixel
                                     qreal curlab[3] = {};
                                     RGB_to_LinearLab(currgb, curlab);
                                     curlab[0] = p / qreal(QuantumRange);
                                     LinearLab_to_RGB(curlab, currgb);
                                     if (pixelEncoding == Photo::HDR) {
                                         pixel[x].red = toHDR(currgb[0]);
                                         pixel[x].green = toHDR(currgb[1]);
                                         pixel[x].blue = toHDR(currgb[2]);
                                     } else if (pixelEncoding == Photo::Linear) {
                                         pixel[x].red = currgb[0];
                                         pixel[x].green = currgb[1];
                                         pixel[x].blue = currgb[2];
                                     } else {
                                         pixel[x].red = invGammaCorrection * pow(currgb[0],1./2.2);
                                         pixel[x].green = invGammaCorrection * pow(currgb[1],1./2.2);
                                         pixel[x].blue = invGammaCorrection * pow(currgb[2],1./2.2);
                                     }
                                 }

                             }
                             cache->sync();
                         });

        return newPhoto;
    }
};

OpMicroContrasts::OpMicroContrasts(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Micro Contrasts"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("Images"),OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
}

OpMicroContrasts *OpMicroContrasts::newInstance()
{
    return new OpMicroContrasts(m_process);
}

OperatorWorker *OpMicroContrasts::newWorker()
{
    return new WorkerMicroContrasts(m_thread, this);
}
