#include "opdeconvolution.h"
#include "workerdeconvolution.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"


OpDeconvolution::OpDeconvolution(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Deconvolution", parent),
    m_luminosity(new OperatorParameterSlider("luminosity", "Luminosity", "Deconvolution Luminosity", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1<<8, 1./(1<<16), 1, Slider::FilterExposureFromOne, this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Kernel","Kernel",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Negative images", "Negative Images", this));
    m_parameters.push_back(m_luminosity);
}

OpDeconvolution *OpDeconvolution::newInstance()
{
    return new OpDeconvolution(m_process);
}

OperatorWorker *OpDeconvolution::newWorker()
{
    return new WorkerDeconvolution(m_luminosity->value(), m_thread, this);
}
