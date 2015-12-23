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
    Operator(OP_SECTION_CURVE, "Shape DR.", Operator::All, parent),
    m_shape(ShapeDynamicRange::TanH),
    m_shapeDialog(new OperatorParameterDropDown("shape", "Shape", this, SLOT(selectShape(int)))),
    m_dynamicRange(new OperatorParameterSlider("dynamicRange", "Dynamic Range", "Shape Dynamic Range", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<12, 1<<10, 1, QuantumRange, Slider::FilterExposure, this)),
    m_exposure(new OperatorParameterSlider("exposure", "Exposure", "Shape Dynamic Range Exposure", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_labDomain(false),
    m_labDomainDialog(new OperatorParameterDropDown("lab", "On L*", this, SLOT(selectLab(int))))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

    m_shapeDialog->addOption("TanH", ShapeDynamicRange::TanH, true);

    m_labDomainDialog->addOption("No", false, true);
    m_labDomainDialog->addOption("Yes", true);

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
    if ( m_labDomain != v ) {
        m_labDomain = v;
        setOutOfDate();
    }
}
