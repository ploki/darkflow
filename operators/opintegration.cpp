#include "process.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "opintegration.h"
#include "workerintegration.h"
#include "Magick++.h"

static const char *RejectionTypeStr[] = {
    "None", "Sigma clipping", "Winsorized", "Median Percentil"
};
static const char *NormalizationTypeStr[] = {
    "None", "Highest Value", "Custom"
};

using Magick::Quantum;

OpIntegration::OpIntegration(Process *parent) :
    Operator(OP_SECTION_BLEND, "Integration", parent),
    m_rejectionType(NoRejection),
    m_rejectionTypeDropDown(new OperatorParameterDropDown("rejectionType", "Rejection", this, SLOT(setRejectionType(int)))),
    m_upper(new OperatorParameterSlider("upper", "Upper mul.", "Integration Upper Limit", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_lower(new OperatorParameterSlider("lower", "Lower div.", "Integration Upper Limit", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_normalizationType(NoNormalization),
    m_normalizationTypeDropDown(new OperatorParameterDropDown("normalizationType", "Normalization", this, SLOT(setNormalizationType(int)))),
    m_customNormalization(new OperatorParameterSlider("normalizationValue", "Custom Norm.", "Integration Custom Normalization", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Integrated Image", "Integrated Image", this));

    m_rejectionTypeDropDown->addOption(RejectionTypeStr[NoRejection], NoRejection, true);
    m_rejectionTypeDropDown->addOption(RejectionTypeStr[SigmaClipping], SigmaClipping);
    m_rejectionTypeDropDown->addOption(RejectionTypeStr[Winsorized], Winsorized);
    m_rejectionTypeDropDown->addOption(RejectionTypeStr[MedianPercentil], MedianPercentil);

    m_normalizationTypeDropDown->addOption(NormalizationTypeStr[NoNormalization], NoNormalization, true);
    m_normalizationTypeDropDown->addOption(NormalizationTypeStr[HighestValue], HighestValue);
    m_normalizationTypeDropDown->addOption(NormalizationTypeStr[Custom], Custom);

    addParameter(m_rejectionTypeDropDown);
    addParameter(m_upper);
    addParameter(m_lower);
    addParameter(m_normalizationTypeDropDown);
    addParameter(m_customNormalization);
}

OpIntegration *OpIntegration::newInstance()
{
    return new OpIntegration(m_process);
}

OperatorWorker *OpIntegration::newWorker()
{
    return new WorkerIntegration(m_rejectionType,
                                 m_upper->value(),
                                 m_lower->value(),
                                 m_normalizationType,
                                 m_customNormalization->value(),
                                 m_thread, this);
}

void OpIntegration::setRejectionType(int type)
{
    if ( m_rejectionType != type ) {
        m_rejectionType = RejectionType(type);
        setOutOfDate();
    }
}

void OpIntegration::setNormalizationType(int type)
{
    if ( m_normalizationType != type ) {
        m_normalizationType = NormalizationType(type);
        setOutOfDate();
    }
}
