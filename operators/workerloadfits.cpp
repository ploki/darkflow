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

WorkerLoadFits::WorkerLoadFits(QVector<QString> filesCollection,
                               OpLoadFits::ColorSpace colorSpace,
                               QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_filesCollection(filesCollection),
    m_colorSpace(colorSpace)
{
}

Photo WorkerLoadFits::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLoadFits::play()
{
    QVector<QString>& collection = m_filesCollection;
    int i, s = collection.count();

    bool failure = false;
#pragma omp parallel for shared(failure) dfl_threads(1)
    for (i = 0 ; i < s ; ++i ) {
        if ( failure || aborted() ) {
            if (!failure)
                dflError(tr("Failed to open %0").arg(collection[i]));
            failure = true;
            continue;

            char cfits_errstatus[31]; //doc says max char is 30
            int status, ret, nfound;
            long naxis[2];
            fitsfile *fptr;
            ret = fits_open_file(&fptr, collection[i].toLocal8Bit().data(), READONLY, &status);
            if ( ret ) {
                fits_get_errstatus(status, cfits_errstatus);
                dflError(tr("fits open failed for %0: %1").arg(collection[i]).arg(cfits_errstatus));
                failure = true;
                continue;
            }

            ret = fits_read_keys_lng(fptr, "NAXIS", 1, 2, &naxis, &nfound, &status);
            if ( ret ) {
                fits_get_errstatus(status, cfits_errstatus);
                dflError(tr("fits read of NAXIS key failed for %0: %1").arg(collection[i]).arg(cfits_errstatus));
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
    }
}
