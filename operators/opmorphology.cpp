/*
 * Copyright (c) 2006-2020, Guillaume Gimenez <guillaume@blackmilk.fr>
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
#include "opmorphology.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "algorithm.h"
#include "hdr.h"

using Magick::Quantum;

class WorkerMorphology : public OperatorWorker {
public:
    WorkerMorphology(OpMorphology::Operation operation,
                     qreal luminosity,
                     QThread *thread,
                     OpMorphology *op) :
        OperatorWorker(thread, op),
        m_operation(operation),
        m_luminosity(luminosity)
    {
    }
    Photo process(const Photo &, int, int) {
        throw 0;
    }
    void play() {
        int c0 = m_inputs[0].count();
        int c1 = m_inputs[1].count();
        for (int i = 0; i < c0 ; ++i) {
            Photo& srcPhoto = m_inputs[0][i];
            Photo& kernelPhoto = m_inputs[1][i%c1];
            Photo outputPhoto(srcPhoto);
            Magick::Image &srcImage = srcPhoto.image();
            Magick::Image &kernelImage = kernelPhoto.image();
            Magick::Image &outputImage = outputPhoto.image();
            ResetImage(outputImage);
            int w = srcImage.columns(),
                h = srcImage.rows();
            int kw = kernelImage.columns(),
                kh = kernelImage.rows();
            std::shared_ptr<Ordinary::Pixels> srcCache(new Ordinary::Pixels(srcImage));
            std::shared_ptr<Ordinary::Pixels> kernelCache(new Ordinary::Pixels(kernelImage));
            std::shared_ptr<Ordinary::Pixels> outputCache(new Ordinary::Pixels(outputImage));
            const Magick::PixelPacket *srcPixels = srcCache->getConst(0, 0, w, h);
            const Magick::PixelPacket *kernelPixels = kernelCache->getConst(0, 0, kw, kh);
            bool srcIsHDR = srcPhoto.getScale() == Photo::HDR;
            bool kernelIsHDR = srcPhoto.getScale() == Photo::HDR;
            dfl_parallel_for(y, 0, h, 4, (outputImage), {
            //for(int y = 0; y < h; ++y ) {
                      Magick::PixelPacket *outputPixels = outputCache->get(0, y, w, 1);
                                 for (int x = 0; x < w; ++x) {
                                     //outputPixels[x] = srcPixels[y*w+x];
                                     double r=1, g=1, b=1;
                                     for (int ky = 0; ky < kh; ++ky) {
                                         for (int kx = 0; kx < kw; ++kx) {
                                             int bx = x - kw/2 + kx;
                                             int by = y - kh/2 + ky;
                                             if (bx < 0 || bx >= w || by < 0 || by >=h )
                                                continue;
                                             double sr, sg, sb;
                                             double kr, kg, kb;
                                             if (srcIsHDR) {
                                                 sr = fromHDR(srcPixels[by*w+bx].red)/QuantumRange;
                                                 sg = fromHDR(srcPixels[by*w+bx].green)/QuantumRange;
                                                 sb = fromHDR(srcPixels[by*w+bx].blue)/QuantumRange;
                                             } else {
                                                 sr = double(srcPixels[by*w+bx].red)/QuantumRange;
                                                 sg = double(srcPixels[by*w+bx].green)/QuantumRange;
                                                 sb = double(srcPixels[by*w+bx].blue)/QuantumRange;
                                             }
                                             if (kernelIsHDR) {
                                                 kr = fromHDR(kernelPixels[ky*kw+kx].red)/QuantumRange;
                                                 kg = fromHDR(kernelPixels[ky*kw+kx].green)/QuantumRange;
                                                 kb = fromHDR(kernelPixels[ky*kw+kx].blue)/QuantumRange;
                                             } else {
                                                 kr = double(kernelPixels[ky*kw+kx].red)/QuantumRange;
                                                 kg = double(kernelPixels[ky*kw+kx].green)/QuantumRange;
                                                 kb = double(kernelPixels[ky*kw+kx].blue)/QuantumRange;
                                             }
                                 #if 0
                                             sr = pow(sr, -M_LN2/log(m_luminosity));
                                             sg = pow(sg, -M_LN2/log(m_luminosity));
                                             sb = pow(sb, -M_LN2/log(m_luminosity));
                                             kr = pow(kr, -M_LN2/log(m_luminosity));
                                             kg = pow(kg, -M_LN2/log(m_luminosity));
                                             kb = pow(kb, -M_LN2/log(m_luminosity));
                                 #else
                                 #endif
                                             switch (m_operation) {
                                     #define DF_NOT(x) (1-(x))
                                     //#define DF_NOT(x) (x==0?1:1./(x*QuantumRange))
                                     #define DF_AND(a,b) sqrt((a)*(b))
                                     #define DF_OR(a,b) DF_NOT(DF_AND(DF_NOT(a), DF_NOT(b)))
                                     //#define DF_AND(a,b) qMin(a,b)
                                     //#define DF_OR(a,b) qMax(a,b)
                                                 // NOT A = 1 - A
                                                 // A AND B = A * B
                                                 // A OR B OR ... = 1 - (1 - A)*(1 - B)*...
                                                 // A -> B = ~A OR B = ~(A AND ~B)
                                                 case OpMorphology::Erode:
                                                 // fits
                                                 // AND/ (S[i,j] -> A[i,j]) = AND/ (~S[i,j] OR A[i,j])
                                                 //                         = AND/ NOT AND(S[i,j], NOT A[i,j])
                                                 //                         = PROD/ 1 - S[i,j] * (1 - A[i,j])
                                                 // 1 - S * (1-A[i,j])
                                                 r*= DF_NOT(DF_AND(m_luminosity*kr, DF_NOT(sr)));
                                                 g*= DF_NOT(DF_AND(m_luminosity*kg, DF_NOT(sg)));
                                                 b*= DF_NOT(DF_AND(m_luminosity*kb, DF_NOT(sb)));
                                                 break;
                                                 case OpMorphology::Dilate:
                                                 // hits
                                                 // OR/ A[i,j] AND S[i,j] = NOT AND/ NOT AND(A[i,j],S[i,j])
                                                 r*= DF_NOT(DF_AND(m_luminosity*sr,kr));
                                                 g*= DF_NOT(DF_AND(m_luminosity*sg,kg));
                                                 b*= DF_NOT(DF_AND(m_luminosity*sb,kb));
                                                 break;
                                             }
                                         }
                                     }
                                     switch (m_operation) {
                                         case OpMorphology::Erode:
                                            break;
                                         break;
                                         case OpMorphology::Dilate:
                                          r = DF_NOT(r);
                                          g = DF_NOT(g);
                                          b = DF_NOT(b);
                                         break;
                                     }
                         #if 0
                                     r =  pow(r, -log(m_luminosity)/M_LN2) * QuantumRange;
                                     g =  pow(g, -log(m_luminosity)/M_LN2) * QuantumRange;
                                     b =  pow(b, -log(m_luminosity)/M_LN2) * QuantumRange;
                         #else
                                     r *= QuantumRange;
                                     g *= QuantumRange;
                                     b *= QuantumRange;
                         #endif
                                     if (srcIsHDR || kernelIsHDR) {
                                         r = toHDR(r);
                                         g = toHDR(g);
                                         b = toHDR(b);
                                     }
                                     outputPixels[x].red = clamp<double>(r, 0, QuantumRange);
                                     outputPixels[x].green = clamp<double>(g, 0, QuantumRange);
                                     outputPixels[x].blue = clamp<double>(b, 0, QuantumRange);
                                 }
                                 outputCache->sync();
                             }
                            );
            emitProgress(i, c0, 1, 1);
            outputPush(0, outputPhoto);
        }
        emitSuccess();
    }

private:
    OpMorphology::Operation m_operation;
    qreal m_luminosity;
};

OpMorphology::OpMorphology(Process *parent) :
    Operator(OP_SECTION_ANALYSIS, QT_TRANSLATE_NOOP("Operator", "Morphology"), Operator::All, parent),
    m_operation(OpMorphology::Erode),
    m_operationDialog(new OperatorParameterDropDown("operation", tr("Operation"), this, SLOT(selectOperation(int)))),
    m_luminosity(new OperatorParameterSlider("luminosity", tr("Luminosity"), tr("Morphology Luminosity"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), QuantumRange, 1, 1./(1<<16), QuantumRange, Slider::FilterExposure, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Kernel"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_operationDialog->addOption(DF_TR_AND_C("Erode"), Erode, true);
    m_operationDialog->addOption(DF_TR_AND_C("Dilate"), Dilate);

    addParameter(m_luminosity);
    addParameter(m_operationDialog);
}

void OpMorphology::selectOperation(int operation) {
    if (m_operation != operation) {
        m_operation = Operation(operation);
        setOutOfDate();
    }
}

OpMorphology *OpMorphology::newInstance()
{
    return new OpMorphology(m_process);
}

OperatorWorker *OpMorphology::newWorker()
{
    return new WorkerMorphology(m_operation, m_luminosity->value(), m_thread, this);
}
