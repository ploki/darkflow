#include "opigamma.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "igamma.h"
#include "preferences.h"

class WorkerIGamma : public OperatorWorker {
public:
    WorkerIGamma(qreal gamma, qreal x0, qreal invert, QThread *thread, OpIGamma *op) :
        OperatorWorker(thread, op),
        m_iGamma(gamma, x0, invert),
        m_invert(invert)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.setScale( m_invert
                           ? Photo::Linear
                           : Photo::NonLinear);
        m_iGamma.applyOn(newPhoto);
        m_iGamma.applyOnImage(newPhoto.curve());
        return newPhoto;
    }
private:
    iGamma m_iGamma;
    bool m_invert;
};

OpIGamma::OpIGamma(Process *parent) :
    Operator(OP_SECTION_CURVE, "iGamma", parent),
    m_gamma(0),
    m_dynamicRange(0),
    m_revert(false),
    m_revertDialog(new OperatorParameterDropDown("revert","Revert", this, SLOT(revert(int))))
{
    qreal defaultGamma;
    qreal defaultDR;
    qreal defaultMax = 1<<12;
    switch (preferences->getCurrentTarget()) {
    case Preferences::Linear:
        defaultGamma = 1;
        defaultMax = defaultDR = 1<<16;
        break;
    case Preferences::sRGB:
        defaultGamma = 2.4;
        defaultDR = 1./0.00304L;
        break;
    case Preferences::IUT_BT_709:
        defaultGamma = 2.222L;
        defaultDR = 1./0.018L;
        break;
    case Preferences::SquareRoot:
        defaultGamma = 2;
        defaultMax = defaultDR = 1<<16;
        break;
    default:
        defaultGamma = 1;
        defaultMax = defaultDR = 1<<16;
        break;
    }

    m_gamma = new OperatorParameterSlider("gamma", "Gamma", "Gamma Power",
                                          Slider::Value, Slider::Logarithmic, Slider::Real,
                                          0.1, 10, defaultGamma, 0.01, 100, Slider::FilterNothing,this);
    m_dynamicRange = new OperatorParameterSlider("logarithmicRange", "Logarithmic on", "Gamma Logarithmic Range",
                                                 Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                                 1, defaultMax, defaultDR, 1, 1<<16, Slider::FilterExposure, this);

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
