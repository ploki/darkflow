#include "opdeconvolution.h"
#include "workerdeconvolution.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"


OpDeconvolution::OpDeconvolution(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Deconvolution", Operator::NonHDR, parent),
    m_luminosity(new OperatorParameterSlider("luminosity", "Luminosity", "Deconvolution Luminosity", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 4, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addInput(new OperatorInput("Kernel","Kernel",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Negative images", "Negative Images", this));
    addParameter(m_luminosity);
}

OpDeconvolution *OpDeconvolution::newInstance()
{
    return new OpDeconvolution(m_process);
}

OperatorWorker *OpDeconvolution::newWorker()
{
    return new WorkerDeconvolution(m_luminosity->value(), m_thread, this);
}
