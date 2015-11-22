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
    m_rejectionTypeDropDown(new OperatorParameterDropDown("rejectionType", "Rejection", RejectionTypeStr[NoRejection],this)),
    m_upper(new OperatorParameterSlider("upper", "Upper mul.", "Integration Upper Limit", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_lower(new OperatorParameterSlider("lower", "Lower div.", "Integration Upper Limit", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_normalizationType(NoNormalization),
    m_normalizationTypeDropDown(new OperatorParameterDropDown("normalizationType", "Normalization", NormalizationTypeStr[NoNormalization], this)),
    m_customNormalization(new OperatorParameterSlider("normalizationValue", "Custom Norm.", "Integration Custom Normalization", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Integrated Image", "Integrated Image", this));

    m_rejectionTypeDropDown->addOption(RejectionTypeStr[NoRejection], this, SLOT(setNoRejection()));
    m_rejectionTypeDropDown->addOption(RejectionTypeStr[SigmaClipping], this, SLOT(setSigmaClip()));
    m_rejectionTypeDropDown->addOption(RejectionTypeStr[Winsorized], this, SLOT(setWinsorized()));
    m_rejectionTypeDropDown->addOption(RejectionTypeStr[MedianPercentil], this, SLOT(setMedianPercentil()));

    m_normalizationTypeDropDown->addOption(NormalizationTypeStr[NoNormalization], this, SLOT(setNoNormalization()));
    m_normalizationTypeDropDown->addOption(NormalizationTypeStr[HighestValue], this, SLOT(setHighestValue()));
    m_normalizationTypeDropDown->addOption(NormalizationTypeStr[Custom], this, SLOT(setCustom()));

    m_parameters.push_back(m_rejectionTypeDropDown);
    m_parameters.push_back(m_upper);
    m_parameters.push_back(m_lower);
    m_parameters.push_back(m_normalizationTypeDropDown);
    m_parameters.push_back(m_customNormalization);
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

void OpIntegration::setNoRejection()
{
    if ( m_rejectionType != NoRejection ) {
        m_rejectionType = NoRejection;
        setOutOfDate();
    }
}

void OpIntegration::setMedianPercentil()
{
    if ( m_rejectionType != MedianPercentil ) {
        m_rejectionType = MedianPercentil;
        setOutOfDate();
    }
}

void OpIntegration::setSigmaClip()
{
    if ( m_rejectionType != SigmaClipping ) {
        m_rejectionType = SigmaClipping;
        setOutOfDate();
    }
}

void OpIntegration::setWinsorized()
{
    if ( m_rejectionType != Winsorized ) {
        m_rejectionType = Winsorized;
        setOutOfDate();
    }
}

void OpIntegration::setNoNormalization()
{
    if ( m_normalizationType != NoNormalization ) {
        m_normalizationType = NoNormalization;
        setOutOfDate();
    }
}

void OpIntegration::setHighestValue()
{
    if ( m_normalizationType != HighestValue ) {
        m_normalizationType = HighestValue;
        setOutOfDate();
    }
}

void OpIntegration::setCustom()
{
    if ( m_normalizationType != Custom ) {
        m_normalizationType = Custom;
        setOutOfDate();
    }
}
