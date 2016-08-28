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
#ifndef WORKERINTEGRATION_H
#define WORKERINTEGRATION_H

#include "operatorworker.h"
#include "opintegration.h"

namespace Magick {
class Image;
}

class WorkerIntegration : public OperatorWorker
{
    Q_OBJECT
public:
    typedef double integration_plane_t;
    WorkerIntegration(OpIntegration::RejectionType rejectionType,
                      qreal upper,
                      qreal lower,
                      OpIntegration::NormalizationType normalizationType,
                      qreal customNormalizationValue,
                      bool outputHDR,
                      qreal scale,
                      QThread *thread, OpIntegration *op);
    ~WorkerIntegration();
    Photo process(const Photo &, int, int) { throw 0; }
    bool play_onInput(int idx);

    QRectF computePlanesDimensions();
signals:

public:
    OpIntegration::RejectionType m_rejectionType;
    qreal m_upper;
    qreal m_lower;
    OpIntegration::NormalizationType m_normalizationType;
    qreal m_customNormalizationValue;
    bool m_outputHDR;
    integration_plane_t *m_integrationPlane;
    int *m_countPlane;
    int m_w;
    int m_h;
    qreal m_offX;
    qreal m_offY;
    qreal m_scale;

private:
    void createPlanes(Magick::Image&);
};

#endif // WORKERINTEGRATION_H
