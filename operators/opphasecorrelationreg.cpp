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
#include "opphasecorrelationreg.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "discretefouriertransform.h"
#include "cielab.h"


class WorkerPhaseCorrelation : public OperatorWorker {
public:
    WorkerPhaseCorrelation(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }

    void play() {
        int count = m_inputs[0].count();
        if ( count != m_inputs[1].count()) {
            emitFailure();
            return;
        }
        DiscreteFourierTransform *dftB = nullptr;
        for (int i = 0 ; i < count ; ++i) {
            if (NULL == dftB) {
                Magick::Image imB = m_inputs[0][i].image();
                imB = DiscreteFourierTransform::roll(imB, imB.columns()/2, imB.rows()/2);
                dftB = new DiscreteFourierTransform(imB, m_inputs[0][i].getScale());
                dftB->conj();
            }
            Magick::Image imA = m_inputs[0][i].image();
            imA = DiscreteFourierTransform::roll(imA, imA.columns()/2, imA.rows()/2);
            DiscreteFourierTransform dftA(imA, m_inputs[0][i].getScale());
            dftA *= (*dftB);
            DiscreteFourierTransform denom(dftA);
            denom.abs();
            dftA /= denom;
            Magick::Image img = dftA.reverse(1, DiscreteFourierTransform::ReverseMagnitude);
            img = DiscreteFourierTransform::roll(img, img.columns()/2, img.rows()/2);
            int w = img.columns();
            int h = img.rows();
            double max = 0;
            int mx, my;
            Ordinary::Pixels cache(img);
            for (int y = 0 ; y < h ; ++y) {
                const Magick::PixelPacket *pixels = cache.getConst(0, y, w, 1);
                for (int x = 0 ; x < w ; ++x ) {
                    quantum_t lm = LUMINANCE_PIXEL(pixels[x]);
                    if ( lm > max ) {
                        mx = x;
                        my = y;
                    }
                }
            }
            Photo registered(m_inputs[0][i]);
            QVector<QPointF> points;
            points.push_back(QPointF(mx, my));
            registered.setPoints(points);
            Photo newPhoto(img, Photo::Linear);
            newPhoto.setIdentity(m_operator->uuid());
            newPhoto.setTag(TAG_NAME, tr("whaou"));
            outputPush(0, registered);
            outputPush(1, newPhoto);
            emitProgress(i, count, 0, 1);
        }
        delete dftB;
        emitSuccess();
    }
};

OpPhaseCorrelationReg::OpPhaseCorrelationReg(Process *parent) :
    Operator(OP_SECTION_REGISTRATION, QT_TRANSLATE_NOOP("Operator","Phase Correlation"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addOutput(new OperatorOutput(tr("Correlation"), this));
}

OpPhaseCorrelationReg *OpPhaseCorrelationReg::newInstance()
{
    return new OpPhaseCorrelationReg(m_process);
}

OperatorWorker *OpPhaseCorrelationReg::newWorker()
{
    return new WorkerPhaseCorrelation(m_thread, this);
}
