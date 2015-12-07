#include "opunsharpmask.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerUnsharpMask : public OperatorWorker {
public:
    WorkerUnsharpMask(qreal radius,
                      qreal sigma,
                      qreal amount,
                      qreal threshold,
                      QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_radius(radius),
        m_sigma(sigma),
        m_amount(amount),
        m_threshold(threshold)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().unsharpmask(m_radius, m_sigma, m_amount, m_threshold);
        return newPhoto;
    }

private:
    qreal m_radius;
    qreal m_sigma;
    qreal m_amount;
    qreal m_threshold;
};

OpUnsharpMask::OpUnsharpMask(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Unsharp Mask", parent),
    m_radius(new OperatorParameterSlider("radius", "Radius", "Unsharp Mask Radius", Slider::Value, Slider::Logarithmic, Slider::Real, .1, 100, 1, .1, 1000, Slider::FilterPixels, this)),
    m_sigma(new OperatorParameterSlider("sigma", "Sigma", "Unsharp Mask Sigma", Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 1, 0, 1, Slider::FilterPercent, this)),
    m_amount(new OperatorParameterSlider("amount", "Amount", "Unsharp Mask Amount", Slider::Percent, Slider::Logarithmic, Slider::Real, 0.1, 10, 1, 0.01, 100, Slider::FilterPercent, this)),
    m_threshold(new OperatorParameterSlider("threshold", "Threshold", "Unsharp Mask Threshold", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange, 1, 1./QuantumRange, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_radius);
    addParameter(m_sigma);
    addParameter(m_amount);
    addParameter(m_threshold);

}

OpUnsharpMask *OpUnsharpMask::newInstance()
{
    return new OpUnsharpMask(m_process);
}

OperatorWorker *OpUnsharpMask::newWorker()
{
    return new WorkerUnsharpMask(m_radius->value(),
                                 m_radius->value()*m_sigma->value(),
                                 m_amount->value(),
                                 m_threshold->value(),
                                 m_thread, this);
}
