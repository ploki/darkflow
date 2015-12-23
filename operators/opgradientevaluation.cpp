#include "opgradientevaluation.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "workergradientevaluation.h"

OpGradientEvaluation::OpGradientEvaluation(Process *parent) :
    Operator(OP_SECTION_COLOR, "Gradient Evaluation", Operator::All, parent),
    m_radius(new OperatorParameterSlider("radius", "Points Radius", "Gradient Evaluation Points Radius", Slider::Value, Slider::Linear, Slider::Real, 0, 20, 3, 0, 100, Slider::FilterPixels, this)),
    m_altitude(new OperatorParameterSlider("altitude", "Altitude", "Gradient Evaluation Altitude", Slider::Value, Slider::Linear, Slider::Real, 0, 100, 0, 0, 10000, Slider::FilterPixels, this)),
    m_pow(new OperatorParameterSlider("pow", "Power", "Gradient Evaluation Power", Slider::Value, Slider::Logarithmic, Slider::Real, .125, 8, .5, 0.01, 50, Slider::FilterNothing, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Gradients", "Gradients", this));
    addParameter(m_radius);
    addParameter(m_altitude);
    addParameter(m_pow);
}

OpGradientEvaluation *OpGradientEvaluation::newInstance()
{
    return new OpGradientEvaluation(m_process);
}

OperatorWorker *OpGradientEvaluation::newWorker()
{
    return new WorkerGradientEvaluation(m_radius->value(),
                                        m_altitude->value(),
                                        m_pow->value(),
                                        m_thread, this);
}
