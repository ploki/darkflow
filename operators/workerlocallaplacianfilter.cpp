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
#include "workerlocallaplacianfilter.h"
#include "photo.h"
#include "console.h"
#include "hdr.h"
#include "gaussianpyramid.h"
#include "laplacianpyramid.h"
#include "cielab.h"

using Magick::Quantum;

#define fsign(x) ((x)>=0?1.:-1.)
#define fd(delta, alpha) pow(delta, alpha)
#define fe(delta, beta) ((delta) * (beta))

static inline
Pyramid::pFloat remap(qreal alpha,
                               qreal beta,
                               qreal sigma,
                               Pyramid::pFloat g0,
                               Pyramid::pFloat i) {
    Pyramid::pFloat p;
    Pyramid::pFloat delta = fabs(i-g0);
    Pyramid::pFloat sign = fsign(i-g0);
    Pyramid::pFloat g = g0;
    if ( delta <= sigma) {
        //rd, fine scale detail
        p = g + sign * sigma * fd(delta/sigma, alpha);
    } else {
        //re, edges
        p = g + sign * ( fe(delta - sigma, beta) + sigma);
    }
    return p;
}

WorkerLocalLaplacianFilter::WorkerLocalLaplacianFilter(qreal alpha, qreal beta, qreal sigma,
                                                       int startLevel, int levelsCount,
                                                       QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_alpha(alpha),
    m_beta(beta),
    m_sigma(sigma),
    m_startLevel(startLevel),
    m_levelsCount(levelsCount)
{}

Photo WorkerLocalLaplacianFilter::process(const Photo &photo, int p, int c) {
    Photo newPhoto(photo);
    Magick::Image& srcImage(const_cast<Magick::Image&>(photo.image()));
    Magick::Image& image(newPhoto.image());
    bool hdr = photo.getScale() == Photo::HDR;
    int originH = image.rows(),
            originW = image.columns();
    Pyramid::pFloat* origin(new Pyramid::pFloat[originH*originW]);
    dfl_block bool error = false;
    std::shared_ptr<Ordinary::Pixels> src_cache(new Ordinary::Pixels(srcImage));
    dfl_parallel_for(y, 0, originH, 4, (srcImage), {
                         const Magick::PixelPacket *pixels = src_cache->getConst(0, y, originW, 1);
                         if (error || !pixels) {
                             if (!error)
                                dflError(DF_NULL_PIXELS);
                             error = true;
                             continue;
                         }
                         for (int x = 0 ; x < originW ; ++x) {
                             if (hdr) {
                                 origin[y*originW+x]=
                                 LUMINANCE(fromHDR(pixels[x].red)/QuantumRange,
                                           fromHDR(pixels[x].green)/QuantumRange,
                                           fromHDR(pixels[x].blue)/QuantumRange);
                             } else {
                                 origin[y*originW+x] =
                                 LUMINANCE(Pyramid::pFloat(pixels[x].red)/QuantumRange,
                                           Pyramid::pFloat(pixels[x].green)/QuantumRange,
                                           Pyramid::pFloat(pixels[x].blue)/QuantumRange);
                             }
                         }
                     });
    GaussianPyramid gaussianPyramid(origin, originW, originH);
    int rootBase = gaussianPyramid.base();
    //compute the laplacian pyramid to have the n - x levels ready to be skipped
    LaplacianPyramid outputLP(gaussianPyramid);
    Pyramid::pFloat* rootImage = gaussianPyramid.getPlane(0);
    int J = gaussianPyramid.levels();

    QRectF roi = photo.getROI().normalized();
    if (roi.isNull()) {
        roi = QRectF(0, 0, originW, originH);
    }

    int l_start = qMin(m_startLevel, J);
    int l_end = qMin(l_start + m_levelsCount, J);

    double estimatedWork = 0;
    for(int l = l_start ; l < l_end ; ++l) {
        int subregion_size = 3 * ((1 << (l + 2)) - 1);
        int gBase = gaussianPyramid.planeBase(l);
        int ROI_X_START = qMax(int(roi.left())>>l, 0);
        int ROI_X_END = qMin(int(roi.right())>>l, gBase);
        int ROI_Y_START = qMax(int(roi.top())>>l, 0);
        int ROI_Y_END = qMin(int(roi.bottom())>>l, gBase);
        estimatedWork +=
                (ROI_X_END-ROI_X_START)*(ROI_Y_END-ROI_Y_START)
                * (3*subregion_size/2) << 1;
    }
    dfl_block double workDone = 0;
    dfl_block int lastEmission = 0;

    dfl_parallel_for(l, l_start, l_end, 1, (), {
        if (aborted()) {
            continue;
        }
        Pyramid::pFloat *outPlane = const_cast<Pyramid::pFloat*>(outputLP.getPlane(l));
        int gBase = gaussianPyramid.planeBase(l);
        int subregion_size = 3 * ((1 << (l + 2)) - 1);
        int subregion_r = subregion_size / 2;

        int ROI_X_START = qMax(int(roi.left())>>l, 0);
        int ROI_X_END = qMin(int(roi.right())>>l, gBase);
        int ROI_Y_START = qMax(int(roi.top())>>l, 0);
        int ROI_Y_END = qMin(int(roi.bottom())>>l, gBase);

        Pyramid::pFloat* remapped(new Pyramid::pFloat[rootBase * rootBase]);
        for (int y = ROI_Y_START ; y < ROI_Y_END ; ++y ) {
            if (aborted()) {
                continue;
            }
            // in the original domain
            int full_res_y = (1 << l) * y;
            if ( full_res_y > rootBase ) {
                continue;
            }
            int roi_y0 = full_res_y - subregion_r - 2;
            int roi_y1 = full_res_y + subregion_r + 2;
            int row_range_start = qMax(0, roi_y0);
            int row_range_end = qMin(roi_y1, rootBase);
            // offset of y in the roi
            int full_res_roi_y = full_res_y - row_range_start;
            //~in the original domain
            for (int x = ROI_X_START ; x < ROI_X_END ; ++x) {
                if (aborted()) {
                    continue;
                }
                int full_res_x = (1 << l) * x;
                if ( full_res_x > rootBase ) {
                    continue;
                }
                int roi_x0 = full_res_x - subregion_r - 2;
                int roi_x1 = full_res_x + subregion_r + 2;
                int col_range_start =qMax(0, roi_x0);
                int col_range_end = qMin(roi_x1, rootBase);
                //offset of x in the roi
                int full_res_roi_x = full_res_x - col_range_start;
                Q_ASSERT(col_range_end <= rootBase);
                Q_ASSERT(row_range_end <= rootBase);

                dfl_critical_section(
                            workDone += (3*subregion_size/2) <<1;
                        int toEmit = 1 + 1000.*workDone/estimatedWork;
                if (toEmit != lastEmission) {
                    emitProgress(p, c,toEmit, 1000);
                    lastEmission = toEmit;
                }
                );
                for (int yy = row_range_start; yy < row_range_end ; ++yy) {
                    for (int xx = col_range_start ; xx < col_range_end ; ++xx) {
                        Pyramid::pFloat *gl0 =
                                const_cast<Pyramid::pFloat*>(gaussianPyramid.getPlane(l));
                        remapped[(yy-row_range_start)*(col_range_end-col_range_start)
                                +(xx-col_range_start)] =
                                remap(m_alpha, m_beta, m_sigma, gl0[y*gBase+x],
                                      rootImage[(yy)*rootBase+(xx)]);
                    }
                }
                //compute sub-pyramid {Ll0[~R0]}
                GaussianPyramid tmpGP(remapped, col_range_end-col_range_start,
                                      row_range_end-row_range_start);
                LaplacianPyramid tmpLP(tmpGP);
                Pyramid::pFloat *lapPlane = tmpLP.getPlane(l);
                int lapbase = tmpLP.planeBase(l);
                Q_ASSERT(lapPlane != nullptr);
                outPlane[y*gBase+x] =
                        lapPlane[((full_res_roi_y)>>l)*lapbase+((full_res_roi_x)>>l)];
            }
        }
        delete[] remapped;
    });
    outputLP.collapse(gaussianPyramid);
    if (!aborted()) {
        int base = outputLP.base();
        Pyramid::pFloat * output = outputLP.getPlane(0);
        ResetImage(image);
        std::shared_ptr<Ordinary::Pixels> dst_cache(new Ordinary::Pixels(image));
        dfl_parallel_for(y, 0, originH, 4, (srcImage, image), {
                             Magick::PixelPacket *pixels = dst_cache->get(0, y, originW, 1);
                             const Magick::PixelPacket *src_pixels = src_cache->getConst(0, y, originW, 1);
                             if (error || !pixels || !src_pixels) {
                                 if (!error)
                                 dflError(DF_NULL_PIXELS);
                                 error = true;
                                 continue;
                             }
                             for (int x = 0 ; x < originW ; ++x) {
                                 Triplet<Pyramid::pFloat> orig;
                                 if (hdr) {
                                     orig.red = fromHDR(src_pixels[x].red)/QuantumRange;
                                     orig.green = fromHDR(src_pixels[x].green)/QuantumRange;
                                     orig.blue = fromHDR(src_pixels[x].blue)/QuantumRange;
                                 } else {
                                     orig.red = Pyramid::pFloat(src_pixels[x].red)/QuantumRange;
                                     orig.green = Pyramid::pFloat(src_pixels[x].green)/QuantumRange;
                                     orig.blue = Pyramid::pFloat(src_pixels[x].blue)/QuantumRange;
                                 }
                                 Pyramid::pFloat cur = LUMINANCE_PIXEL(orig);
                                 Pyramid::pFloat lum = output[y*base+x]*QuantumRange;
                                 orig *= (lum/cur);

                                 if (hdr) {
                                     pixels[x].red = toHDR(orig.red);
                                     pixels[x].green = toHDR(orig.green);
                                     pixels[x].blue = toHDR(orig.blue);
                                 } else {
                                     pixels[x].red = clamp<quantum_t>(orig.red);
                                     pixels[x].green = clamp<quantum_t>(orig.green);
                                     pixels[x].blue = clamp<quantum_t>(orig.blue);
                                 }
                             }
                             dst_cache->sync();
                         });
    }
    delete[] origin;
    return newPhoto;
}
