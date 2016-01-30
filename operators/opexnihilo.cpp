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
#include <QVector>
#include "operatoroutput.h"
#include "opexnihilo.h"
#include "operatorworker.h"
#include "process.h"
#include "photo.h"
#include "console.h"

#include <Magick++.h>

class ExNihilo : public OperatorWorker
{
public:
    ExNihilo(QThread *thread, OpExNihilo *op) :
        OperatorWorker(thread, op)
    {}
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play() {
        dflDebug("play!!");
        Photo photo(Photo::Linear);
        photo.setIdentity(m_operator->uuid());
        photo.createImage(1000,1000);
        if (photo.isComplete()) {
            try {
                Magick::Image& image = photo.image();
                Magick::Pixels cache(image);
                unsigned w = image.columns();
                unsigned h = image.rows();
                for (unsigned y = 0 ; y < h ; ++y) {
                    emit progress(y, h);
                    if ( aborted() ) {
                        emitFailure();
                        return;
                    }
                    Magick::PixelPacket *pixels = cache.get(0,y,w,1);
                    if ( m_error || !pixels ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for (unsigned x = 0 ; x < w ; ++x ) {
                        using Magick::Quantum;
                        pixels[x].red = qrand()%QuantumRange;
                        pixels[x].green = qrand()%QuantumRange;
                        pixels[x].blue = qrand()%QuantumRange;
                    }
                    cache.sync();
                }
                photo.setTag(TAG_NAME, "Random Image");
                outputPush(0, photo);
                emitSuccess();
            }
            catch(std::exception &e) {
                dflError("%s", e.what());
                emitFailure();
            }
        }
        else {
            emitFailure();
        }
    }
};

OpExNihilo::OpExNihilo(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, QT_TRANSLATE_NOOP("Operator", "Ex Nihilo"), Operator::NA, parent)
{
    addOutput(new OperatorOutput(tr("Random image"), this));
}

OpExNihilo::~OpExNihilo()
{
    //dflDebug((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}

OpExNihilo *OpExNihilo::newInstance()
{
    return new OpExNihilo(m_process);
}

OperatorWorker *OpExNihilo::newWorker()
{
    return new ExNihilo(m_thread, this);
}

