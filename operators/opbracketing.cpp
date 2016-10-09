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
#include "opbracketing.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "photo.h"
#include "ports.h"
#include "cielab.h"
#include "hdr.h"
#include <Magick++.h>

class WorkerBracketing : public OperatorWorker {
public:
    WorkerBracketing(qreal compensation, qreal high, qreal low,
                     bool automatic,
                     QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_compensation(compensation),
        m_high(high),
        m_low(low),
        m_automatic(automatic)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        newPhoto.setTag(TAG_HDR_COMP,QString::number(m_compensation));
        newPhoto.setTag(TAG_HDR_HIGH,QString::number(m_high));
        newPhoto.setTag(TAG_HDR_LOW,QString::number(m_low));
        newPhoto.setTag(TAG_HDR_AUTO, "0");
        return newPhoto;
    }
    double mean(Photo& photo) {
        Magick::Image& image = photo.image();
        int w = image.columns(),
            h = image.rows();
        double lum = 0;
        bool hdr = photo.getScale() == Photo::HDR;
        Ordinary::Pixels cache(image);
        for (int y = 0 ; y < h ; ++y) {
            const Magick::PixelPacket *pixels = cache.getConst(0, y, w, 1);
            for (int x = 0 ; x < w ; ++x) {
                lum += LUMINANCE(
                            hdr?fromHDR(pixels[x].red):pixels[x].red,
                            hdr?fromHDR(pixels[x].green):pixels[x].green,
                            hdr?fromHDR(pixels[x].blue):pixels[x].blue);
            }
        }
        return lum / (w*h);
    }

    void play() {
        if (!m_automatic)
            return OperatorWorker::play();
        int i = 0, s = m_inputs[0].count();
        if ( 0 == s ) {
            emitSuccess();
            return;
        }
        Photo *reference = Photo::findReference(m_inputs[0]);
        double refMean;
        if (reference)
            refMean = mean(*reference);
        else
            refMean = mean(m_inputs[0][0]);
        foreach(Photo photo, m_inputs[0]) {
            double curMean = mean(photo);
            Photo newPhoto(photo);
            double compensation = curMean/refMean;
            newPhoto.setTag(TAG_HDR_COMP,QString::number(compensation));
            newPhoto.setTag(TAG_HDR_HIGH,QString::number(m_high));
            newPhoto.setTag(TAG_HDR_LOW,QString::number(m_low));
            newPhoto.setTag(TAG_HDR_AUTO, "1");
            outputPush(0, newPhoto);
            emitProgress(i++, s, 0, 1);
        }
        emitSuccess();
    }

private:
    qreal m_compensation, m_high, m_low;
    bool m_automatic;
};

OpBracketing::OpBracketing(Process *parent) :
    Operator(OP_SECTION_BLEND, QT_TRANSLATE_NOOP("Operator", "Bracketing"), Operator::All, parent),
    m_compensation(new OperatorParameterSlider("compensation", tr("Compensation"), tr("Bracketing Exposure Compensation"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<16, 1, 1, 1<<16, Slider::FilterExposureFromOne, this)),
    m_high(new OperatorParameterSlider("high", tr("Limit High"), tr("Bracketing Limit High"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, M_SQRT1_2l, 1./(1<<16), 1<<16, Slider::FilterExposureFromOne, this)),
    m_low(new OperatorParameterSlider("low", tr("Limit Low"), tr("Bracketing Limit Low"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1./(1<<8), 1./(1<<16), 1<<16, Slider::FilterExposureFromOne, this)),
    m_automatic(new OperatorParameterDropDown("automatic", tr("Automatic"), this, SLOT(selectAutomatic(int)))),
    m_automaticValue(false)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_automatic->addOption(DF_TR_AND_C("No"), false, true);
    m_automatic->addOption(DF_TR_AND_C("Yes"), true);

    addParameter(m_compensation);
    addParameter(m_high);
    addParameter(m_low);
    addParameter(m_automatic);
}

OpBracketing *OpBracketing::newInstance()
{
    return new OpBracketing(m_process);
}

OperatorWorker *OpBracketing::newWorker()
{
    return new WorkerBracketing(m_compensation->value(),
                                m_high->value(),
                                m_low->value(),
                                m_automaticValue,
                                m_thread, this);
}

void OpBracketing::selectAutomatic(int v)
{
    if ( m_automaticValue != !!v ) {
        m_automaticValue = !!v;
        setOutOfDate();
    }
}
