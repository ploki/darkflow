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
    m_dynamicRange(new OperatorParameterSlider("logarithmicRange", "Logarithmic on", "Gamma Logarithmic Range",
                                               Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                               1, 1<<12, 1./0.00304L, 1, 1<<16, Slider::FilterExposure, this)),
    m_revert(false),
    m_revertDialog(new OperatorParameterDropDown("revert","Revert", this, SLOT(revert(int))))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

    m_revertDialog->addOption("No", false, true);
    m_revertDialog->addOption("Yes", true);

    addParameter(m_gamma);
    addParameter(m_dynamicRange);
    addParameter(m_revertDialog);

}

OpIGamma *OpIGamma::newInstance()
{
    return new OpIGamma(m_process);
}

OperatorWorker *OpIGamma::newWorker()
{
    return new WorkerIGamma(m_gamma->value(), 1./m_dynamicRange->value(), m_revert, m_thread, this);
}

void OpIGamma::revert(int v)
{
    if ( m_revert != !!v ) {
        m_revert = !!v;
        setOutOfDate();
    }
}
