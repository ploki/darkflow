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
#include <list>

#include "workerloadimage.h"


WorkerLoadImage::WorkerLoadImage(QVector<QString> filesCollection,
                                 OpLoadImage::ColorSpace colorSpace,
                                 QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_filesCollection(filesCollection),
    m_colorSpace(colorSpace)
{
}

Photo WorkerLoadImage::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLoadImage::play()
{
    QVector<QString>& collection = m_filesCollection;

    int s = collection.count();
    dfl_block int p = 0;
    dfl_block bool failure = false;

    dfl_parallel_for(i, 0, s, 1, (), {
        if ( failure || aborted() ) {
            failure = true;
            continue;
        }
        try {
            QString filename=collection[i];
            std::list<Magick::Image> images;
            Magick::readImages(&images, filename.toStdString());
            Photo::Gamma gamma;
            switch(m_colorSpace) {
            default:
            case OpLoadImage::Linear: gamma = Photo::Linear; break;
            case OpLoadImage::IUT_BT_709: gamma = Photo::IUT_BT_709; break;
            case OpLoadImage::sRGB: gamma = Photo::sRGB; break;
            case OpLoadImage::HDR: gamma = Photo::HDR; break;
            }
            int plane = 0;
            int count = images.size();
            for( std::list<Magick::Image>::iterator it = images.begin() ;
                 it != images.end() ;
                 ++it ) {
                Photo photo(*it, gamma);
                if ( !photo.isComplete() ) {
                    failure = true;
                    continue;
                }
                QFileInfo finfo(collection[i]);
                QString identity = finfo.fileName();
                if ( count > 1 )
                    identity += ":" + QString::number(plane);
                photo.setIdentity(m_operator->uuid()+"/"+identity);
                photo.setTag(TAG_NAME, identity);
                photo.setSequenceNumber(i);
                photo.setTag(TAG_SCALE, gamma == Photo::Linear
                             ? TAG_SCALE_LINEAR
                             : TAG_SCALE_NONLINEAR);

                dfl_critical_section({
                    outputPush(0, photo);
                });
                ++plane;
            }
        }
        catch(std::exception &e) {
            dflError("%s", e.what());
            failure = true;
        }
        catch(...) {
            dflError(tr("Unknown image exception"));
            failure = true;
        }

        dfl_critical_section({
            if ( !failure )
                emit progress(++p, s);
        });
    });
    if ( failure ) {
        emitFailure();
    }
    else {
        outputSort(0);
        emitSuccess();
    }

}
