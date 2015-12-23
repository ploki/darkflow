#include "opcolorfilter.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "colorfilter.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerColorFilter : public OperatorWorker {
public:
    WorkerColorFilter(qreal r, qreal g, qreal b, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_colorFilter(r,g,b)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_colorFilter.applyOn(newPhoto);
        return newPhoto;
    }

private:
    ColorFilter m_colorFilter;
};

OpColorFilter::OpColorFilter(Process *parent) :
    Operator(OP_SECTION_COLOR, "Color Filter", Operator::All, parent),
    m_r(new OperatorParameterSlider("red", "Red", "Color Filter Red Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_g(new OperatorParameterSlider("green", "Green", "Color Filter Green Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_b(new OperatorParameterSlider("blue", "Blue", "Color Filter Blue Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_r);
    addParameter(m_g);
    addParameter(m_b);
}

OpColorFilter *OpColorFilter::newInstance()
{
    return new OpColorFilter(m_process);
}

OperatorWorker *OpColorFilter::newWorker()
{
    return new WorkerColorFilter(m_r->value(), m_g->value(), m_b->value(), m_thread, this);
}
