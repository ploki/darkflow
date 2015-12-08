#include "opconvolution.h"
#include "workerconvolution.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"


OpConvolution::OpConvolution(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Convolution", parent),
    m_luminosity(new OperatorParameterSlider("luminosity", "Luminosity", "Convolution Luminosity", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 4, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addInput(new OperatorInput("Kernel","Kernel",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Negative images", "Negative Images", this));
    addParameter(m_luminosity);
}

OpConvolution *OpConvolution::newInstance()
{
    return new OpConvolution(m_process);
}

OperatorWorker *OpConvolution::newWorker()
{
    return new WorkerConvolution(m_luminosity->value(), m_thread, this);
}
