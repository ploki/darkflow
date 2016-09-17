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
#include "opcmycompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "algorithm.h"
#include "console.h"
#include "cielab.h"
#include "hdr.h"
#include "transformview.h"

class WorkerCMYCompose : public OperatorWorker {
    bool m_outputHDR;
public:
    WorkerCMYCompose(bool outputHDR, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_outputHDR(outputHDR)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int l_count = m_inputs[0].count();
        int c_count = m_inputs[1].count();
        int m_count = m_inputs[2].count();
        int y_count = m_inputs[3].count();
        int photo_count = qMax(qMax(qMax(l_count, c_count), m_count), y_count);

        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pLuminance;
            Photo pCyan;
            Photo pMagenta;
            Photo pYellow;
            Photo *photos[4] = {};
            std::shared_ptr<TransformView> tvLuminance(0);
            std::shared_ptr<TransformView> tvCyan(0);
            std::shared_ptr<TransformView> tvMagenta(0);
            std::shared_ptr<TransformView> tvYellow(0);
            unsigned w = 0, h = 0;
            QVector<QPointF> reference;

            if ( l_count ) {
                pLuminance = m_inputs[0][i%l_count];
                photos[0] = &pLuminance;
            }

            if ( c_count ) {
                pCyan = m_inputs[1][i%c_count];
                photos[2] = &pCyan;
            }

            if ( m_count ) {
                pMagenta = m_inputs[2][i%m_count];
                photos[3] = &pMagenta;
            }

            if ( y_count ) {
                pYellow = m_inputs[3][i%y_count];
                photos[1] = &pYellow;
            }

            Photo *refPhoto = Photo::findReference(photos, sizeof(photos)/sizeof(*photos));
            if (refPhoto) {
                reference = refPhoto->getPoints();
                w = refPhoto->image().columns();
                h = refPhoto->image().rows();
            }
            if ( l_count ) {
                tvLuminance.reset(new TransformView(pLuminance, 1, reference));
                if ( tvLuminance->inError() || !tvLuminance->loadPixels()) {
                    m_error = true;
                    continue;
                }
            }
            if ( c_count ) {
                tvCyan.reset(new TransformView(pCyan, 1, reference));
                if ( tvCyan->inError() || !tvCyan->loadPixels()) {
                    m_error = true;
                    continue;
                }
            }
            if ( m_count ) {
                tvMagenta.reset(new TransformView(pMagenta, 1, reference));
                if ( tvMagenta->inError() || !tvMagenta->loadPixels()) {
                    m_error = true;
                    continue;
                }
            }
            if ( y_count ) {
                tvYellow.reset(new TransformView(pYellow, 1, reference));
                if ( tvYellow->inError() || !tvYellow->loadPixels()) {
                    m_error = true;
                    continue;
                }
            }

            try {
                Photo photo(Photo::Linear);
                photo.createImage(w, h);
                photo.setSequenceNumber(i);
                photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
                photo.setTag(TAG_NAME, "LCMY Composition");
                Ordinary::Pixels iPhoto_cache(photo.image());
                Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
                dfl_block int line = 0;
                bool hdrLuminance = pLuminance.getScale() == Photo::HDR;
                bool hdrCyan = pCyan.getScale() == Photo::HDR;
                bool hdrMagenta = pMagenta.getScale() == Photo::HDR;
                bool hdrYellow = pYellow.getScale() == Photo::HDR;
                dfl_parallel_for(y, 0, int(h), 4, (), {
                    if ( m_error ) {
                        continue;
                    }
                    for ( unsigned x = 0 ; x < w ; ++x ) {
                        double rgb[3];
                        double cyan=0, magenta=0, yellow=0;
                        if (tvCyan) {
                            Magick::PixelPacket pxl = tvCyan->getPixel(x, y, 0);
                            if (hdrCyan) {
                                cyan = fromHDR(pxl.green) + fromHDR(pxl.blue) / 2.;
                            }
                            else {
                                cyan = double(pxl.green) + double(pxl.blue) / 2.;
                            }
                        }
                        if (tvMagenta) {
                            Magick::PixelPacket pxl = tvMagenta->getPixel(x, y, 0);
                            if (hdrMagenta) {
                                magenta = fromHDR(pxl.red) + fromHDR(pxl.blue) / 2.;
                            }
                            else {
                                magenta = double(pxl.red) + double(pxl.blue) / 2.;
                            }
                        }
                        if (tvYellow) {
                            Magick::PixelPacket pxl = tvYellow->getPixel(x, y, 0);
                            if (hdrYellow) {
                                yellow = fromHDR(pxl.green) + fromHDR(pxl.red) / 2.;
                            }
                            else {
                                yellow = double(pxl.green) + double(pxl.red) / 2.;
                            }
                        }

                        rgb[0] =  - cyan + magenta + yellow;
                        rgb[1] =    cyan - magenta + yellow;
                        rgb[2] =    cyan + magenta - yellow;

                        if (tvLuminance) {
                            Magick::PixelPacket pixel = tvLuminance->getPixel(x, y, 0);
                            double lum = LUMINANCE(hdrLuminance?fromHDR(pixel.red):pixel.red,
                                                   hdrLuminance?fromHDR(pixel.green):pixel.green,
                                                   hdrLuminance?fromHDR(pixel.blue):pixel.blue);
                            double cur = LUMINANCE(rgb[0],
                                                   rgb[1],
                                                   rgb[2]);
                            double mul = lum/cur;
                            rgb[0] = mul*rgb[0];
                            rgb[1] = mul*rgb[1];
                            rgb[2] = mul*rgb[2];
                        }
                        if (m_outputHDR) {
                            pxl[y*w+x].red = toHDR(rgb[0]);
                            pxl[y*w+x].green = toHDR(rgb[1]);
                            pxl[y*w+x].blue = toHDR(rgb[2]);
                        }
                        else {
                            pxl[y*w+x].red=clamp<quantum_t>(DF_ROUND(rgb[0]));
                            pxl[y*w+x].green=clamp<quantum_t>(DF_ROUND(rgb[1]));
                            pxl[y*w+x].blue=clamp<quantum_t>(DF_ROUND(rgb[2]));
                        }
                    }
                    dfl_critical_section(
                    {
                        emitProgress(i, photo_count,line++, h);
                    });
                });
                iPhoto_cache.sync();
                if (m_outputHDR)
                    photo.setScale(Photo::HDR);
                outputPush(0, photo);
                emitProgress(i, photo_count, 1, 1);
            }
            catch (std::exception &e) {
                if (l_count)
                    setError(pLuminance, e.what());
                if (c_count)
                    setError(pCyan, e.what());
                if (m_count)
                    setError(pMagenta, e.what());
                if (y_count)
                    setError(pYellow, e.what());
                emitFailure();
                return;
            }
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }
};

OpCMYCompose::OpCMYCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "LCMY Compose"), Operator::NonHDR, parent),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    addInput(new OperatorInput(tr("Luminance"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Cyan"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Magenta"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Yellow"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("RGB"), this));

    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);
    addParameter(m_outputHDR);
}

OpCMYCompose *OpCMYCompose::newInstance()
{
    return new OpCMYCompose(m_process);
}

OperatorWorker *OpCMYCompose::newWorker()
{
    return new WorkerCMYCompose(m_outputHDRValue, m_thread, this);
}

void OpCMYCompose::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}
