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
#include "opline.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "operatorparameterslider.h"

using Magick::Quantum;

class WorkerLine : public OperatorWorker {
    quantum_t m_color;
    qreal m_diameter;
    bool m_keepBackground;
    OpLine::DirectionType m_direction;
    qreal m_period;
    qreal m_gap;
    qreal m_decay;
    int m_skip;
public:
    WorkerLine(quantum_t color, qreal diameter,
               bool keepBackground, OpLine::DirectionType direction,
               qreal period, qreal gap, qreal decay, int skip,
               QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
      m_color(color),
      m_diameter(diameter),
      m_keepBackground(keepBackground),
      m_direction(direction),
      m_period(period),
      m_gap(gap),
      m_decay(decay),
      m_skip(skip)
    {
    }

    void straightLine(Magick::PixelPacket *dstPixels, int w, int h, const QPointF& p,
                      qreal gap, qreal decay, int i) {
        int ymin = p.y()-m_diameter/2,
            ymax = p.y()+m_diameter/2,
            xmin = p.x()-m_diameter/2,
            xmax = p.x()+m_diameter/2;
        //qreal pixelDecay = i==0 ? 1 : pow(decay, i);
        qreal pixelDecay = pow(decay, i);
        qDebug("pixelDecay=%f, i=%d", pixelDecay, i);
        switch(m_direction) {
        case OpLine::Vertical:
            /* pd
             *  .9^0 = 1
             *  .9^1 = .9
             *  .9^2 = .81
             *  height = pd * h
             *  height/2 = pd * h/2
             *  margin = h - pd * h
             *  margin/2 = h * (1-pd)/2
             */
            ymin = 0+(1.-pixelDecay)*h/2.; ymax = h-(1.-pixelDecay)*h/2.;
            xmin = qMax(0, xmin); xmax = qMin(w, xmax);
            break;
        case OpLine::Horizontal:
            xmin = 0+(1.-pixelDecay)*w/2.; xmax = w-(1.-pixelDecay)*w/2.;
            ymin = qMax(0, ymin); ymax = qMin(h, ymax);
        }
        qDebug("xmin=%d, xmax=%d, ymin=%d, ymax=%d",xmin,xmax,ymin,ymax);
        for (int x = xmin ; x < xmax ; ++x ) {
            for (int y = ymin ; y < ymax ; ++y) {
                if ( pow(x-(w/2), 2) + pow(y-(h/2),2) > pow(gap, 2) )
                    dstPixels[y*w+x].red = dstPixels[y*w+x].green = dstPixels[y*w+x].blue  = m_color;
            }
        }
    }

    Photo process(const Photo &photo, int , int ) {
        Photo srcPhoto(photo);
        Photo dstPhoto(photo);
        Magick::Image& srcImage = srcPhoto.image();
        Magick::Image& dstImage = dstPhoto.image();
        int w = dstImage.columns(),
            h = dstImage.rows();
        ResetImage(dstImage);
        std::shared_ptr<Ordinary::Pixels> srcCache(new Ordinary::Pixels(srcImage));
        Ordinary::Pixels dstCache(dstImage);
        Magick::PixelPacket *dstPixels = dstCache.get(0, 0, w, h);
        quantum_t bgColor = QuantumRange-m_color;
        Magick::PixelPacket bgPacket;
        bgPacket.red = bgPacket.green = bgPacket.blue = bgColor;
        dfl_parallel_for(y, 0, h, 4, (srcImage), {
                             const Magick::PixelPacket *srcPixels = srcCache->getConst(0, y, w, 1);
                             for (int x = 0 ; x < w ; ++x) {
                                 dstPixels[y*w+x] = m_keepBackground
                                                    ? srcPixels[x]
                                                    : bgPacket;
                             }
                         });
        QVector<QPointF> points = srcPhoto.getPoints();
        if (m_period >= 1) {
            qreal m = m_direction == OpLine::Vertical ? w/2. : h/2.;
            if (m_period == 0)
                m_period = 1;
            qDebug("m_skip=%d", m_skip);
            for (int i = m_skip ; m+(m_period * i) < m * 2. ; i++) {
                straightLine(dstPixels, w, h,
                            QPoint((m+m_period*i) * m_direction,
                                   (m+m_period*i) * (1-m_direction)),
                             m_gap, m_decay, i);
                if (i != 0 ) //not doing this line twice
                    straightLine(dstPixels, w, h,
                                QPoint((m-m_period*i) * m_direction,
                                       (m-m_period*i) * (1-m_direction)),
                                 m_gap, m_decay, i);
            }
        }
        if (!points.empty()) {
            for (int i = 0, s = points.count() ; i < s ; ++i ) {
                straightLine(dstPixels, w, h, points[i], m_gap, m_decay, 0);
            }
        }
        dstCache.sync();
        dstPhoto.setPoints(QVector<QPointF>());
        return dstPhoto;
    }

};

OpLine::OpLine(Process *parent) :
    Operator(OP_SECTION_MASK, QT_TRANSLATE_NOOP("Operator", "Line"), Operator::All, parent),
    m_color(new OperatorParameterDropDown("color", tr("Color"), this, SLOT(selectColor(int)))),
    m_colorValue(QuantumRange),
    m_diameter(new OperatorParameterSlider("diameter", tr("Thickness"), tr("Line - Thickness in pixels"), Slider::Value, Slider::Linear, Slider::Real, 0, 1000, 200, 0, 16384, Slider::FilterPixels, this)),
    m_keepBackground(new OperatorParameterDropDown("keepBackground", tr("Keep Background"),this, SLOT(selectKeepBackground(int)))),
    m_keepBackgroundValue(true),
    m_direction(new OperatorParameterDropDown("direction", tr("Direction"),this, SLOT(selectDirection(int)))),
    m_directionValue(Vertical),
    m_period(new OperatorParameterSlider("period", tr("Period"), tr("Line - Period in pixels"), Slider::Value, Slider::Linear, Slider::Real, 0, 2000, 0, 0, 16384, Slider::FilterPixels, this)),
    m_gap(new OperatorParameterSlider("gap", tr("Center gap"), tr("Line - Center gap for DC"), Slider::Value, Slider::Linear, Slider::Real, 0, 1000, 200, 0, 16384, Slider::FilterPixels, this)),
    m_decay(new OperatorParameterSlider("decay", tr("Decay"), tr("Line - Decay"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 0, 0, 1, Slider::FilterPercent, this)),
    m_skip(new OperatorParameterSlider("skip", tr("Skip period"), tr("Line - Skip #th first periods"), Slider::Value, Slider::Linear, Slider::Real, 0, 5, 0, 0, 16384, Slider::FilterPixels, this))
{
        addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
        addOutput(new OperatorOutput(tr("Images"), this));

        m_color->addOption(DF_TR_AND_C("White"), QuantumRange, true);
        m_color->addOption(DF_TR_AND_C("Black"), 0);

        m_keepBackground->addOption(DF_TR_AND_C("No"), false);
        m_keepBackground->addOption(DF_TR_AND_C("Yes"), true, true);

        m_direction->addOption(DF_TR_AND_C("Vertical"), Vertical, true);
        m_direction->addOption(DF_TR_AND_C("Horizontal"), Horizontal);

        addParameter(m_color);
        addParameter(m_diameter);
        addParameter(m_keepBackground);
        addParameter(m_direction);
        addParameter(m_period);
        addParameter(m_gap);
        addParameter(m_decay);
        addParameter(m_skip);
}

OpLine *OpLine::newInstance()
{
    return new OpLine(m_process);
}

OperatorWorker *OpLine::newWorker()
{
    return new WorkerLine(m_colorValue,
                          m_diameter->value(),
                          m_keepBackgroundValue,
                          m_directionValue,
                          m_period->value(),
                          m_gap->value(),
                          m_decay->value(),
                          m_skip->value(),
                          m_thread, this);
}

void OpLine::selectColor(int v)
{
    if (m_colorValue != v) {
        m_colorValue = v;
        setOutOfDate();
    }
}

void OpLine::selectKeepBackground(int v)
{
    if (m_keepBackgroundValue != !!v) {
        m_keepBackgroundValue = !!v;
        setOutOfDate();
    }
}

void OpLine::selectDirection(int v)
{
    if (m_directionValue != DirectionType(v)) {
        m_directionValue = DirectionType(v);
        setOutOfDate();
    }
}
