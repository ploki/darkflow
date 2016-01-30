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
#include "opshapedynamicrange.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "shapedynamicrange.h"
#include "photo.h"
#include <Magick++.h>

using Magick::Quantum;
class WorkerShapeDR : public OperatorWorker {
public:
    WorkerShapeDR(ShapeDynamicRange::Shape shape,
                  qreal dynamicRange,
                  qreal exposure,
                  bool labDomain,
                  QThread *thread,
                  OpShapeDynamicRange *op) :
        OperatorWorker(thread, op),
        m_shapeDR(shape, dynamicRange, exposure, labDomain)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_shapeDR.applyOn(newPhoto);
        return newPhoto;
    }

private:
    ShapeDynamicRange m_shapeDR;
};

OpShapeDynamicRange::OpShapeDynamicRange(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "Shape DR."), Operator::All, parent),
    m_shape(ShapeDynamicRange::TanH),
    m_shapeDialog(new OperatorParameterDropDown("shape", tr("Shape"), this, SLOT(selectShape(int)))),
    m_dynamicRange(new OperatorParameterSlider("dynamicRange", tr("Dynamic Range"), tr("Shape Dynamic Range"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<12, 1<<10, 1, QuantumRange, Slider::FilterExposure, this)),
    m_exposure(new OperatorParameterSlider("exposure", tr("Exposure"), tr("Shape Dynamic Range Exposure"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_labDomain(false),
    m_labDomainDialog(new OperatorParameterDropDown("lab", tr("On Luminance"), this, SLOT(selectLab(int))))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    m_shapeDialog->addOption(DF_TR_AND_C("TanH"), ShapeDynamicRange::TanH, true);

    m_labDomainDialog->addOption(DF_TR_AND_C("No"), false, true);
    m_labDomainDialog->addOption(DF_TR_AND_C("Yes"), true);

    addParameter(m_shapeDialog);
    addParameter(m_dynamicRange);
    addParameter(m_exposure);
    addParameter(m_labDomainDialog);

}

OpShapeDynamicRange *OpShapeDynamicRange::newInstance()
{
    return new OpShapeDynamicRange(m_process);
}

OperatorWorker *OpShapeDynamicRange::newWorker()
{
  return new WorkerShapeDR(m_shape, m_dynamicRange->value(), m_exposure->value(), m_labDomain, m_thread, this);
}

void OpShapeDynamicRange::selectShape(int shape)
{
    if ( m_shape != shape ) {
        m_shape = ShapeDynamicRange::Shape(shape);
        setOutOfDate();
    }
}

void OpShapeDynamicRange::selectLab(int v)
{
    if ( m_labDomain != !!v ) {
        m_labDomain = !!v;
        setOutOfDate();
    }
}
