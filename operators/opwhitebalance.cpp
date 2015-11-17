#include "opwhitebalance.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "whitebalance.h"

class WorkerWhiteBalance : public OperatorWorker {
public:
    WorkerWhiteBalance(qreal temperature, qreal tint, bool safe, QThread *thread, OpWhiteBalance *op) :
        OperatorWorker(thread, op),
        m_whitebalance(temperature,tint, safe)
    {}
    Photo process(const Photo& photo, int, int) {
        return m_whitebalance.apply(photo);
    }

private:
    WhiteBalance m_whitebalance;
};

OpWhiteBalance::OpWhiteBalance(Process *parent) :
    Operator(OP_SECTION_CORRECTION, "White Balance", parent),
    m_temperature(new OperatorParameterSlider("temperature", "Temperature", "White Balance Temperature",
                                              Slider::Value, Slider::Logarithmic, Slider::Integer,
                                              2000, 12000, 6500, 2000, 12000, Slider::FilterNothing,this)),
    m_tint(new OperatorParameterSlider("tint", "Green tint", "White Balance Green Tint",
                                       Slider::Percent, Slider::Logarithmic, Slider::Real,
                                       0.5, 2, 1, 0.01, 100, Slider::FilterNothing, this)),
    m_safe(false),
    m_safeDialog(new OperatorParameterDropDown("safe","Range safe", "No", this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));

    m_safeDialog->addOption("No", this, SLOT(setUnsafe()));
    m_safeDialog->addOption("Yes", this, SLOT(setSafe()));

    m_parameters.push_back(m_temperature);
    m_parameters.push_back(m_tint);
    m_parameters.push_back(m_safeDialog);

}

OpWhiteBalance *OpWhiteBalance::newInstance()
{
    return new OpWhiteBalance(m_process);
}

OperatorWorker *OpWhiteBalance::newWorker()
{
    return new WorkerWhiteBalance(m_temperature->value(), m_tint->value(), m_safe, m_thread, this);
}

void OpWhiteBalance::setUnsafe()
{
    if ( m_safe ) {
        m_safe = false;
        setUpToDate(false);
    }
}

void OpWhiteBalance::setSafe()
{
    if ( !m_safe ) {
        m_safe = true;
        setUpToDate(false);
    }
}
