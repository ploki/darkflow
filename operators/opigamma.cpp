#include "opigamma.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "igamma.h"

class WorkerIGamma : public OperatorWorker {
public:
    WorkerIGamma(qreal gamma, qreal x0, qreal invert, QThread *thread, OpIGamma *op) :
        OperatorWorker(thread, op),
        m_iGamma(gamma, x0, invert)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_iGamma.applyOn(newPhoto);
        m_iGamma.applyOnImage(newPhoto.curve());
        return newPhoto;
    }
private:
    iGamma m_iGamma;
};

OpIGamma::OpIGamma(Process *parent) :
    Operator(OP_SECTION_CURVE, "iGamma", parent),
    m_gamma(new OperatorParameterSlider("gamma", "Gamma", "Gamma Power",
                                        Slider::Value, Slider::Logarithmic, Slider::Real,
                                        0.1, 10, 2.4, 0.01, 100, Slider::FilterNothing,this)),
    m_dynamicRange(new OperatorParameterSlider("dynamicRange", "Dynamic Range", "Gamma Dynamic Range",
                                               Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                               1, 1<<12, 1./0.00304L, 1, 1<<16, Slider::FilterExposure, this)),
    m_revert(false),
    m_revertDialog(new OperatorParameterDropDown("revert","Revert", "No", this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));

    m_revertDialog->addOption("No", this, SLOT(revertNo()));
    m_revertDialog->addOption("Yes", this, SLOT(revertYes()));

    m_parameters.push_back(m_gamma);
    m_parameters.push_back(m_dynamicRange);
    m_parameters.push_back(m_revertDialog);

}

OpIGamma *OpIGamma::newInstance()
{
    return new OpIGamma(m_process);
}

OperatorWorker *OpIGamma::newWorker()
{
    return new WorkerIGamma(m_gamma->value(), 1./m_dynamicRange->value(), m_revert, m_thread, this);
}

void OpIGamma::revertYes()
{
    if ( !m_revert ) {
        m_revert = true;
        setUpToDate(false);
    }
}

void OpIGamma::revertNo()
{
    if ( m_revert ) {
        m_revert = false;
        setUpToDate(false);
    }
}