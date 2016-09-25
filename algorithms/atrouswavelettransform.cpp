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
#include "atrouswavelettransform.h"
#include "hdr.h"
#include <Magick++.h>
#include "console.h"

using Magick::Quantum;

static void
kernel_1D_to_2D(const double *in, double *out, int kOrder)
{
    double sum = 0;
    for (int i = 0 ; i < kOrder ; ++i) {
        for (int j = 0 ; j < kOrder ; ++j) {
            //Kronecker product
            sum += out[i*kOrder+j] = in[i]*in[j];
        }
    }
    for (int i = 0, s = kOrder * kOrder ; i < s ; ++i) {
        out[i]/=sum;
    }
}


ATrousWaveletTransform::ATrousWaveletTransform(Photo &photo,
                                               const double *kernel,
                                               int kOrder) :
    m_w(photo.image().columns()),
    m_h(photo.image().rows()),
    m_image(new Triplet<double>[m_w*m_h]),
    m_tmp(new Triplet<double>[m_w*m_h]),
    m_kOrder(kOrder),
    m_kernel(new double[kOrder*kOrder]),
    m_identity(photo.getIdentity()),
    m_name(photo.getTag(TAG_NAME))
{
    kernel_1D_to_2D(kernel, m_kernel, m_kOrder);
    Magick::Image& image = photo.image();
    bool hdr = photo.getScale() == Photo::HDR;
    std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(image));
    dfl_block bool error = false;
    dfl_parallel_for(y, 0, m_h, 4, (image), {
                         const Magick::PixelPacket *pixels = cache->getConst(0, y, m_w, 1);
                         if ( error || !pixels ) {
                             if (!error)
                                dflError(DF_NULL_PIXELS);
                             error=true;
                             continue;
                         }
                         for (int x = 0 ; x < m_w ; ++x ) {
                             if (hdr) {
                                 m_image[y*m_w+x].red = fromHDR(pixels[x].red);
                                 m_image[y*m_w+x].green = fromHDR(pixels[x].green);
                                 m_image[y*m_w+x].blue = fromHDR(pixels[x].blue);
                             }
                             else {
                                 m_image[y*m_w+x].red = pixels[x].red;
                                 m_image[y*m_w+x].green = pixels[x].green;
                                 m_image[y*m_w+x].blue = pixels[x].blue;
                             }
                         }
                     });
}

ATrousWaveletTransform::~ATrousWaveletTransform()
{
    delete[] m_image;
    delete[] m_tmp;
    delete[] m_kernel;
}

Photo ATrousWaveletTransform::transform(int n, int nPlanes, Photo::Gamma scale, Photo &sign)
{
    ResetImage(sign.image());
    Magick::Image &iSign = sign.image();
    Photo plane(scale);
    plane.createImage(m_w, m_h);
    Magick::Image &iPlane = plane.image();
    iPlane.modifyImage();
    bool outputHDR = scale == Photo::HDR;
    std::shared_ptr<Ordinary::Pixels> cSign(new Ordinary::Pixels(iSign));
    std::shared_ptr<Ordinary::Pixels> cPlane(new Ordinary::Pixels(iPlane));
    int spread = 1<<n;
    int halfOrder = m_kOrder/2;
    bool lastPlane = (n == nPlanes - 1);
    dfl_parallel_for(y, 0, m_h, 4, (iSign, iPlane), {
                         Magick::PixelPacket *pSign = cSign->get(0, y, m_w, 1);
                         Magick::PixelPacket *pPlane = cPlane->get(0, y, m_w, 1);
                         for ( int x = 0 ; x < m_w ; ++x ) {
                             Triplet<double> pixel, p;
                             if (!lastPlane) {
                                 for (int i = 0 ; i < m_kOrder ; ++i) {
                                     for (int j = 0 ; j < m_kOrder ; ++j ) {
                                         double coef = m_kernel[i*m_kOrder+j];
                                         int yy = clamp<int>(y+(i-halfOrder)*spread, 0, m_h-1);
                                         int xx = clamp<int>(x+(j-halfOrder)*spread, 0, m_w-1);
                                         p = m_image[yy*m_w+xx];
                                         p *= coef;
                                         pixel += p;
                                     }
                                 }
                                 m_tmp[y*m_w+x] = pixel;
                                 pixel-=m_image[y*m_w+x];
                                 pixel*=-1;
                             }
                             else {
                                 pixel = m_image[y*m_w+x];
                             }
                             pSign[x].red = pSign[x].green = pSign[x].blue = 0;
                             if (pixel.red < 0) {
                                 pSign[x].red = QuantumRange;
                                 pixel.red = -pixel.red;
                             }
                             if (pixel.green < 0) {
                                 pSign[x].green = QuantumRange;
                                 pixel.green = -pixel.green;
                             }
                             if (pixel.blue < 0) {
                                 pSign[x].blue = QuantumRange;
                                 pixel.blue = -pixel.blue;
                             }
                             if (outputHDR) {
                                 pPlane[x].red = toHDR(pixel.red);
                                 pPlane[x].green = toHDR(pixel.green);
                                 pPlane[x].blue = toHDR(pixel.blue);
                             }
                             else {
                                 pPlane[x].red = pixel.red;
                                 pPlane[x].green = pixel.green;
                                 pPlane[x].blue = pixel.blue;
                             }
                         }
                         cSign->sync();
                         cPlane->sync();
                     });
    if (!lastPlane)
        for (int i=0, s = m_w*m_h ; i < s ; ++i)
            m_image[i] = m_tmp[i];
    plane.setIdentity(m_identity+QString(":W:%0").arg(n+1));
    plane.setTag(TAG_NAME, m_name+QString(":W:%0").arg(n+1));
    sign.setIdentity(m_identity+QString(":S:%0").arg(n+1));
    sign.setTag(TAG_NAME, m_name+QString(":S:%0").arg(n+1));

    return plane;
}

void ATrousWaveletTransform::construct(Photo &plane, int n)
{
    Q_UNUSED(plane);
    Q_UNUSED(n)
    throw 0;
}

Photo ATrousWaveletTransform::getConstruction()
{
    throw 0;
}
