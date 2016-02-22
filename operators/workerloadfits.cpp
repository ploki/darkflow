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
#include <QFileInfo>

#include <fitsio.h>

#include "workerloadfits.h"
#include "photo.h"
#include "hdr.h"

WorkerLoadFits::WorkerLoadFits(QVector<QString> filesCollection,
                               OpLoadFits::ColorSpace colorSpace,
                               bool outputHDR,
                               QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_filesCollection(filesCollection),
    m_colorSpace(colorSpace),
    m_outputHDR(outputHDR)
{
}

Photo WorkerLoadFits::process(const Photo &, int, int)
{
    throw 0;
}

static Photo::Gamma getGamma(WorkerLoadFits *worker, OpLoadFits::ColorSpace space) {
    switch (space) {
    default:
        worker->dflWarning(WorkerLoadFits::tr("Unknown color space"));
    case OpLoadFits::Linear:
        return Photo::Linear;
    case OpLoadFits::sRGB:
        return Photo::sRGB;
    case OpLoadFits::IUT_BT_709:
        return Photo::IUT_BT_709;
    }
}

void WorkerLoadFits::play()
{
    QVector<QString>& collection = m_filesCollection;
    int i, s = collection.count();
    int seq=0;
    bool failure = false;
//#pragma omp parallel for shared(failure) dfl_threads(1)
    for (i = 0 ; i < s ; ++i ) {
        if ( failure || aborted() ) {
            if (!failure)
                dflError(tr("Failed to open %0").arg(collection[i]));
            failure = true;
            continue;
        }
        char cfits_errstatus[31]; //doc says max char is 30
        int status = 0, ret, nfound;
        long naxis[2];
        fitsfile *fptr = NULL;
        ret = fits_open_file(&fptr, collection[i].toLocal8Bit().data(), READONLY, &status);
        if ( ret ) {
            fits_get_errstatus(status, cfits_errstatus);
            dflError(tr("fits open failed for %0: %1").arg(collection[i]).arg(cfits_errstatus));
            failure = true;
            continue;
        }

        int ii, hdutype;

        for ( ii = 1; !(fits_movabs_hdu(fptr, ii, &hdutype, &status)); ++ii ) {

            int nkeys, keypos;
            ret = fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxis, &nfound, &status);
            if ( ret ) {
                fits_get_errstatus(status, cfits_errstatus);
                dflError(tr("fits read of NAXIS key failed for %0: %1").arg(collection[i]).arg(cfits_errstatus));
                failure = true;
                status = 0;
                break;
            }

            if (nfound !=2) {
                dflWarning(tr("fits file is not 2-D: %0").arg(collection[i]));
                continue;
            }
            Photo photo(getGamma(this, m_colorSpace));
            photo.createImage(naxis[0], naxis[1]);

            float buf[4096];
            float nullval;
            int npixels = naxis[0] * naxis[1];
            int fpixels = 1;
            int nbuf;
            int anynull;

            float d_max = 1e-30;
            float d_min = 1e30;

            Magick::Image &image = photo.image();
            Magick::Pixels image_cache(image);
            Magick::PixelPacket *pixels = image_cache.get(0, 0, naxis[0], naxis[1]);
            dflInfo(tr("x: %0, y: %1").arg(naxis[0]).arg(naxis[1]));
            while ( npixels > 0 ) {
                nbuf = npixels;
                if ( nbuf > int(sizeof(buf)/sizeof(*buf)) )
                    nbuf = sizeof(buf)/sizeof(*buf);
                ret = fits_read_img(fptr, TFLOAT, fpixels, nbuf, &nullval,
                                    buf, &anynull, &status);
                if ( ret ) {
                    fits_get_errstatus(status, cfits_errstatus);
                    dflError(tr("fits read img failed for %0: %1").arg(collection[i]).arg(cfits_errstatus));
                    failure = true;
                    status = 0;
                    break;
                }
                for ( int j = 0 ; j < nbuf ; ++j ) {
                    long off = j+fpixels;
                    if ( off > naxis[0] * naxis[1] ) {
                        break;
                    }
                    pixels[off].red = pixels[off].green = pixels[off].blue =
                            m_outputHDR ? toHDR(buf[j]) : quantum_t(buf[j]);
                    if ( buf[j] > d_max )
                        d_max = buf[j];
                    if ( buf[j] < d_min )
                        d_min = buf[j];
                }
                npixels-=nbuf;
                fpixels+=nbuf;
            }
            dflInfo(tr("pixels read: %0").arg(fpixels));
            dflInfo(tr("dmin: %0, dmax: %1").arg(d_min).arg(d_max));
            if ( ret )
                break;
            image_cache.sync();

            QFileInfo finfo(collection[i]);
            QString identity = finfo.fileName();
            photo.setIdentity(m_operator->uuid()+"/"+identity);
            photo.setTag(TAG_NAME, identity);
            photo.setSequenceNumber(seq++);
            photo.setTag(TAG_SCALE, m_outputHDR ? TAG_SCALE_HDR :
                                                  getGamma(this, m_colorSpace) == Photo::Linear
                                                  ? TAG_SCALE_LINEAR
                                                  : TAG_SCALE_NONLINEAR);

            outputPush(0, photo);
            ret = fits_get_hdrpos(fptr, &nkeys, &keypos, &status);
            if ( ret ) {
                fits_get_errstatus(status, cfits_errstatus);
                dflError(tr("fits get hdrpos for %0: %1").arg(collection[i]).arg(cfits_errstatus));
                failure = true;
                status = 0;
                break;
            }


        }
        if ( END_OF_FILE == status ) {
            status = 0;
        }
        else {
            fits_get_errstatus(status, cfits_errstatus);
            dflError(tr("fits failure for %0: %1").arg(collection[i]).arg(cfits_errstatus));
            failure = true;
            continue;
        }


        ret = fits_close_file(fptr, &status);
        if ( ret ) {
            fits_get_errstatus(status, cfits_errstatus);
            dflError(tr("fits close failed for %0: %1").arg(collection[i]).arg(cfits_errstatus));
            failure = true;
            continue;
        }

    }
    emitSuccess();
}
