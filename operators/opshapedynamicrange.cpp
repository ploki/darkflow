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
    Operator(OP_SECTION_CURVE, "Shape DR.", parent),
    m_shape(ShapeDynamicRange::TanH),
    m_shapeDialog(new OperatorParameterDropDown("shape", "Shape", "TanH", this)),
    m_dynamicRange(new OperatorParameterSlider("dynamicRange", "Dynamic Range", "Shape Dynamic Range", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<12, 1<<10, 1, QuantumRange, Slider::FilterExposure, this)),
    m_exposure(new OperatorParameterSlider("exposure", "Exposure", "Shape Dynamic Range Exposure", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_labDomain(false),
    m_labDomainDialog(new OperatorParameterDropDown("lab", "On L*", "No", this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));

    m_shapeDialog->addOption("TanH", this, SLOT(selectShapeTanH()));

    m_labDomainDialog->addOption("No", this, SLOT(selectLabNo()));
    m_labDomainDialog->addOption("Yes", this, SLOT(selectLabYes()));

    m_parameters.push_back(m_shapeDialog);
    m_parameters.push_back(m_dynamicRange);
    m_parameters.push_back(m_exposure);
    m_parameters.push_back(m_labDomainDialog);

}

OpShapeDynamicRange *OpShapeDynamicRange::newInstance()
{
    return new OpShapeDynamicRange(m_process);
}

OperatorWorker *OpShapeDynamicRange::newWorker()
{
  return new WorkerShapeDR(m_shape, m_dynamicRange->value(), m_exposure->value(), m_labDomain, m_thread, this);
}

void OpShapeDynamicRange::selectShapeTanH()
{
    if ( m_shape != ShapeDynamicRange::TanH ) {
        m_shape = ShapeDynamicRange::TanH;
        setUpToDate(false);
    }
}

void OpShapeDynamicRange::selectLabNo()
{
    if ( m_labDomain ) {
        m_labDomain = false;
        setUpToDate(false);
    }
}

void OpShapeDynamicRange::selectLabYes()
{
    if ( !m_labDomain ) {
        m_labDomain = true;
        setUpToDate(false);
    }
}
