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
        m_colorFilter.applyOnImage(newPhoto.curve());
        return newPhoto;
    }

private:
    ColorFilter m_colorFilter;
};

OpColorFilter::OpColorFilter(Process *parent) :
    Operator(OP_SECTION_COLOR, "Color Filter", parent),
    m_r(new OperatorParameterSlider("red", "Red", "Color Filter Red Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_g(new OperatorParameterSlider("green", "Green", "Color Filter Green Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this)),
    m_b(new OperatorParameterSlider("blue", "Blue", "Color Filter Blue Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposure, this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));
    m_parameters.push_back(m_r);
    m_parameters.push_back(m_g);
    m_parameters.push_back(m_b);
}

OpColorFilter *OpColorFilter::newInstance()
{
    return new OpColorFilter(m_process);
}

OperatorWorker *OpColorFilter::newWorker()
{
    return new WorkerColorFilter(m_r->value(), m_g->value(), m_b->value(), m_thread, this);
}
