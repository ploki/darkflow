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
#include "opmicrocontrasts.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

static void
normalizeKernel(int order, double *kernel)
{
  double div = 0;
  int i;
  int n = order * order;
  for ( i = 0 ; i < n ; ++i )
    div+=kernel[i];
  for ( i = 0 ; i < n ; ++i )
    kernel[i]/=div;
}

class WorkerMicroContrasts : public OperatorWorker {
public:
    WorkerMicroContrasts(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        double kernel[]={ 0 , -1.,  0,
                         -1.,  8., -1.,
                          0 , -1.,  0};
        normalizeKernel(3, kernel);
        newPhoto.image().convolve(3, kernel);
        return newPhoto;
    }
};

OpMicroContrasts::OpMicroContrasts(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Micro Contrasts"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput(tr("Images"),OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
}

OpMicroContrasts *OpMicroContrasts::newInstance()
{
    return new OpMicroContrasts(m_process);
}

OperatorWorker *OpMicroContrasts::newWorker()
{
    return new WorkerMicroContrasts(m_thread, this);
}
