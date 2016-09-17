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
#include "operatorworker.h"
#include "oprotate.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "photo.h"
#include <Magick++.h>
#include <QTransform>

class RotateWorker : public OperatorWorker {
public:
    RotateWorker(QThread *thread, Operator *op) :
        OperatorWorker(thread, op) {}
    Photo process(const Photo& Photo, int p, int c) Q_DECL_OVERRIDE;
};

Photo RotateWorker::process(const Photo& photo, int p, int c) {
    Q_UNUSED(p);
    Q_UNUSED(c);
    Photo newPhoto(photo);
    qreal angle = dynamic_cast<OpRotate*>(m_operator)->angle();
    //qreal rad = M_PI*angle/180.;
    QTransform transform;

    qreal w = double(newPhoto.image().columns())/2,
            h = double(newPhoto.image().rows())/2;

    dflDebug(tr("apply rotation of %0 °").arg(angle));
    newPhoto.image().rotate(angle);
    //read in reverse order
    //transform.translate(w*cos(rad)+h*sin(rad), -w*sin(rad)+h*cos(rad));
    transform.rotate(angle);
    //transform.translate(-w, -h);
    QVector<QPointF> points = newPhoto.getPoints(),
            newPoints;
    qreal x,y, xmin = 0, ymin = 0;
    transform.map(0, 0, &x, &y); if (x < xmin) xmin = x; if (y < ymin) ymin = y;
    transform.map(0, h, &x, &y); if (x < xmin) xmin = x; if (y < ymin) ymin = y;
    transform.map(w, 0, &x, &y); if (x < xmin) xmin = x; if (y < ymin) ymin = y;
    transform.map(w, h, &x, &y); if (x < xmin) xmin = x; if (y < ymin) ymin = y;
    for ( int i = 0, s = points.count() ; i < s ; ++i ) {
        qreal x, y;
        transform.map(points[i].x(), points[i].y(), &x, &y);
        newPoints.push_back(QPointF(x-2*xmin,y-2*ymin));
    }
    newPhoto.setPoints(newPoints);
    return newPhoto;
}

OpRotate::OpRotate(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, QT_TRANSLATE_NOOP("Operator", "Rotation"), Operator::All, parent),
    m_dropdown(new OperatorParameterDropDown("angle", tr("Angle"), this, SLOT(setAngle(int)))),
    m_angle(0)
{
    m_dropdown->addOption(DF_TR_AND_C("0°"), 0, true);
    m_dropdown->addOption(DF_TR_AND_C("90°"), 90);
    m_dropdown->addOption(DF_TR_AND_C("180°"), 180);
    m_dropdown->addOption(DF_TR_AND_C("270°"), 270);
    addParameter(m_dropdown);
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Rotated"), this));
}

OpRotate::~OpRotate()
{
}

OpRotate *OpRotate::newInstance()
{
    return new OpRotate(m_process);
}

OperatorWorker *OpRotate::newWorker()
{
    return new RotateWorker(m_thread, this);
}

void OpRotate::setAngle(int v)
{
    if ( m_angle != v ) {
        m_angle = v;
        setOutOfDate();
    }
}

qreal OpRotate::angle() const
{
    return m_angle;
}
