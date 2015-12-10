#include "oplimereg.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "workerlimereg.h"

OpLimereg::OpLimereg(Process *parent) :
    Operator(OP_SECTION_REGISTRATION, "LimeReg", parent),
    m_maxRotation(new OperatorParameterSlider("maxRotation", "Max Rotation", "LimeReg Max Rotation", Slider::Value, Slider::Linear, Slider::Real, 0, 180, 15, 0, 180, Slider::FilterPixels, this)),
    m_maxTranslationPercent(new OperatorParameterSlider("maxTranslation", "Max Translation", "LimeReg Max Translation", Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .1, 0, 1, Slider::FilterPercent, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_maxRotation);
    addParameter(m_maxTranslationPercent);
}

OpLimereg *OpLimereg::newInstance()
{
    return new OpLimereg(m_process);
}

OperatorWorker *OpLimereg::newWorker()
{
    return new WorkerLimereg(m_maxRotation->value(),
                             m_maxTranslationPercent->value() * 100,
                             m_thread, this);
}
