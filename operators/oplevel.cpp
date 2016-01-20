#include "oplevel.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"

using Magick::Quantum;

class WorkerLevel : public OperatorWorker {
public:
    WorkerLevel(qreal blackPoint, qreal whitePoint, qreal gamma, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_blackPoint(blackPoint),
        m_whitePoint(whitePoint),
        m_gamma(gamma)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().level(m_blackPoint*QuantumRange,
                               m_whitePoint*QuantumRange,
                               m_gamma);
        newPhoto.curve().level(m_blackPoint*QuantumRange,
                               m_whitePoint*QuantumRange,
                               m_gamma);
        return newPhoto;
    }

private:
    qreal m_blackPoint;
    qreal m_whitePoint;
    qreal m_gamma;
};

OpLevel::OpLevel(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "Level"), Operator::NonHDR, parent),
    m_blackPoint(new OperatorParameterSlider("blackPoint", "Black Point", "Level Black Point", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1./(1<<16), 1./(1<<16), 1, Slider::FilterExposureFromOne, this)),
    m_whitePoint(new OperatorParameterSlider("blackPoint", "White Point", "Level White Point", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1, 1./(1<<16), 1, Slider::FilterExposureFromOne, this)),
    m_gamma(new OperatorParameterSlider("gamma", "Gamma", "Level Gamma", Slider::Value, Slider::Logarithmic, Slider::Real, 0.1, 10, 1, 0.01, 10, Slider::FilterNothing, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_blackPoint);
    addParameter(m_whitePoint);
    addParameter(m_gamma);
}

OpLevel *OpLevel::newInstance()
{
    return new OpLevel(m_process);
}

OperatorWorker *OpLevel::newWorker()
{
return new WorkerLevel(m_blackPoint->value(),
                       m_whitePoint->value(),
                       m_gamma->value(),
                       m_thread, this);
}
