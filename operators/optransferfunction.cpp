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
#include "optransferfunction.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "Magick++.h"
#include "photo.h"
#include "exposure.h"
#include "hdr.h"

using Magick::Quantum;

// M : [0..Q] -> [0..1]
static qreal mapQuantum(Photo::Gamma g, quantum_t q) {
    if (g == Photo::HDR)
        return fromHDR(q)/QuantumRange;
    if (g == Photo::NonLinear)
        return pow(qreal(q)/QuantumRange, 2.2);
    return qreal(q);
}

class WorkerTransferFunction : public OperatorWorker {
    int m_width;
    bool m_grid;
    OpTransferFunction::Encoding m_encoding;
    OpTransferFunction::Encoding m_scale;
public:
    WorkerTransferFunction(int width, bool grid, OpTransferFunction::Encoding encoding,
                           OpTransferFunction::Encoding scale,
                           QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_width(width),
        m_grid(grid),
        m_encoding(encoding),
        m_scale(scale)
    {}
    Photo process(const Photo&, int, int) { throw 0; };

    void play() {
        int photo_count = m_inputs[0].count();
        if (m_inputs[1].count() !=  photo_count) {
            dflError("Odd number of inputs");
            emitFailure();
            return;
        }
        for (int i = 0; i < photo_count; ++i) {
            if (aborted())
                continue;
            Photo Xphoto = m_inputs[0][i];
            Photo Yphoto = m_inputs[1][i];
            Photo Hphoto(Photo::Linear);
            Photo::Gamma Xg = Xphoto.getScale();
            Photo::Gamma Yg = Xphoto.getScale();
            Hphoto.setIdentity(m_operator->uuid());
            Hphoto.setTag(TAG_NAME, "H:X→Y");
            Hphoto.setScale(Photo::Linear);
            Hphoto.createImage(m_width, m_width);
            Magick::Image& Himage = Hphoto.image();
            Magick::Image& Ximage = Xphoto.image();
            Magick::Image& Yimage = Yphoto.image();
            Ordinary::Pixels Hcache(Himage);
            Ordinary::Pixels Xcache(Ximage);
            Ordinary::Pixels Ycache(Yimage);
            Magick::PixelPacket *Hpixels = Hcache.get(0,0, m_width, m_width);
            size_t w = Ximage.columns(),
                h = Ximage.rows();
            if (w != Yimage.columns() || h != Yimage.rows()) {
                dflError("Odd image sizes");
                continue;
            }
            QVector<qreal> Hmap(m_width*m_width*3);
            qreal maxPopulation[3] = {};
            for (size_t y = 0; y < h; ++y) {
                const Magick::PixelPacket *Xpixels = Xcache.getConst(0,y,w,1);
                const Magick::PixelPacket *Ypixels = Ycache.getConst(0,y,w,1);
                for (size_t x = 0; x < w; ++x) {
                    quantum_t Xq[3] = {Xpixels[x].red, Xpixels[x].green, Xpixels[x].blue};
                    quantum_t Yq[3] = {Ypixels[x].red,Ypixels[x].green,Ypixels[x].blue};
                    quantum_t Hx[3], Hy[3];
                    for (int c = 0; c < 3; ++c) {
                        qreal X = mapQuantum(Xg, Xq[c]);
                        qreal Y = mapQuantum(Yg, Yq[c]);
                        if (m_scale == OpTransferFunction::Gamma) {
                            X = pow(X, 1./2.2);
                            Y = pow(Y, 1./2.2);
                        } else if (m_scale == OpTransferFunction::Log && X != 0) {
                            //X = log(X*QuantumRange)/log(2)/16;
                            X = log(X*QuantumRange)/log(QuantumRange);
                            Y = log(Y*QuantumRange)/log(QuantumRange);
                        }
                        Hx[c] = clamp<quantum_t>(m_width * X, 0, m_width - 1);
                        Hy[c] = clamp<quantum_t>(m_width * Y, 0, m_width - 1);
                        int hindex = (Hy[c]*m_width+Hx[c])+c*m_width*m_width;
                        Hmap[hindex] += 1;
                        if ( Hmap[hindex] > maxPopulation[c])
                            maxPopulation[c] = Hmap[hindex];
                    }
                    if (0) {
                        Hpixels[Hy[0]*m_width+Hx[0]].red = Hmap[(Hy[0]*m_width+Hx[0])+0*m_width*m_width];
                        Hpixels[Hy[1]*m_width+Hx[1]].green = Hmap[(Hy[1]*m_width+Hx[1])+1*m_width*m_width];
                        Hpixels[Hy[2]*m_width+Hx[2]].blue = Hmap[(Hy[2]*m_width+Hx[2])+2*m_width*m_width];
                    }
                }
            }
            if (1)
                for(int y = 0; y < m_width; ++y) {
                    for(int x = 0; x < m_width; ++x) {
                        for(int c = 0; c < 3; ++c) {
                            int hindex = (y*m_width+x)+c*m_width*m_width;
                            qreal t = Hmap[hindex];
                            if (m_encoding == OpTransferFunction::Log && t != 0) {
                                Hmap[hindex] = log(t)/log(maxPopulation[c])*QuantumRange;
                            } else if (m_encoding == OpTransferFunction::Linear) {
                                Hmap[hindex] = t/maxPopulation[c]*QuantumRange;
                            } else if (m_encoding == OpTransferFunction::Gamma) {
                                Hmap[hindex] = pow(t/maxPopulation[c],1./2.2)*QuantumRange;
                            }

                        }
                        Hpixels[y*m_width+x].red = Hmap[(y*m_width+x)+0*m_width*m_width];
                        Hpixels[y*m_width+x].green = Hmap[(y*m_width+x)+1*m_width*m_width];
                        Hpixels[y*m_width+x].blue = Hmap[(y*m_width+x)+2*m_width*m_width];
                    }
                }

            if (m_grid)
                for(int l = 0; l < 16; ++l) {
                    int offset = 0;
                    if (m_scale == OpTransferFunction::Linear) {
                        offset = m_width * pow(2, l) / QuantumRange;
                    } else if (m_scale == OpTransferFunction::Log) {
                        offset = m_width * l / 16;
                    } else if (m_scale == OpTransferFunction::Gamma) {
                        offset = m_width * pow(qreal(l)/16, 2.2);
                    }
                    for (int k = 0; k < m_width; ++k) {
                        for (int c = 0; c < 3; ++c) {
                            Hpixels[k+offset*m_width].red = Hpixels[k+offset*m_width].green = Hpixels[k+offset*m_width].blue = QuantumRange/32;
                            Hpixels[offset+k*m_width].red = Hpixels[offset+k*m_width].green = Hpixels[offset+k*m_width].blue = QuantumRange/32;
                        }
                    }
                }
            Hcache.sync();
            Himage.flip();
            outputPush(0, Hphoto);
            emitProgress(i, photo_count, 1, 1);
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }

};

OpTransferFunction::OpTransferFunction(Process *parent) :
    Operator(OP_SECTION_ANALYSIS, QT_TRANSLATE_NOOP("Operator", "Transfer Function"), Operator::All, parent),
    m_widthDialog(new OperatorParameterSlider("width",tr("Width"), tr("Transfer Function - Image width"), Slider::Value, Slider::Linear, Slider::Integer, 0, 1024, 512, 0, 65535, Slider::FilterPixels, this)),
    m_gridDialog(new OperatorParameterDropDown("grid", tr("Grid"), this, SLOT(selectGrid(int)))),
    m_grid(false),
    m_encodingDialog(new OperatorParameterDropDown("encoding", tr("Encoding"), this, SLOT(selectEncoding(int)))),
    m_encoding(Gamma),
    m_scaleDialog(new OperatorParameterDropDown("scale", tr("Scale"), this, SLOT(selectScale(int)))),
    m_scale(Linear)
{
    addInput(new OperatorInput(tr("X"),OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Y"),OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("H:X→Y"), this));

    m_encodingDialog->addOption(DF_TR_AND_C("I"), Identity);
    m_encodingDialog->addOption(DF_TR_AND_C("∝"), Linear);
    m_encodingDialog->addOption(DF_TR_AND_C("γ"), Gamma, true);
    m_encodingDialog->addOption(DF_TR_AND_C("Log"), Log);
    m_scaleDialog->addOption(DF_TR_AND_C("∝"), Linear);
    m_scaleDialog->addOption(DF_TR_AND_C("γ"), Gamma);
    m_scaleDialog->addOption(DF_TR_AND_C("Log"), Log, true);
    m_gridDialog->addOption(DF_TR_AND_C("Off"), false, true);
    m_gridDialog->addOption(DF_TR_AND_C("On"), true);

    addParameter(m_widthDialog);
    addParameter(m_gridDialog);
    addParameter(m_encodingDialog);
    addParameter(m_scaleDialog);
}

OpTransferFunction *OpTransferFunction::newInstance()
{
    return new OpTransferFunction(m_process);
}

OperatorWorker *OpTransferFunction::newWorker()
{
    return new WorkerTransferFunction(m_widthDialog->value(),
                                      m_grid,
                                      m_encoding,
                                      m_scale,
                                      m_thread,
                                      this);
}

void OpTransferFunction::selectEncoding(int v)
{
    if (m_encoding != v) {
        m_encoding = Encoding(v);
        setOutOfDate();
    }
}

void OpTransferFunction::selectScale(int v)
{
    if (m_scale != v) {
        m_scale = Encoding(v);
        setOutOfDate();
    }
}

void OpTransferFunction::selectGrid(int v)
{
    if (m_grid != !!v) {
        m_grid = !!v;
        setOutOfDate();
    }
}
