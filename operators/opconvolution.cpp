#include "opconvolution.h"
#include "workerconvolution.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"


OpConvolution::OpConvolution(Process *parent) :
    Operator(OP_SECTION_EFFECTS, QT_TRANSLATE_NOOP("Operator", "Convolution"), Operator::NonHDR, parent),
    m_luminosity(new OperatorParameterSlider("luminosity", tr("Luminosity"), tr("Convolution Luminosity"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 4, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Kernel"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
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
