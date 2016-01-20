#include "opblur.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerBlur : public OperatorWorker {
public:
    WorkerBlur(qreal radius,
               qreal sigma,
               QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_radius(radius),
        m_sigma(sigma)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().blur(m_radius, m_sigma);
        return newPhoto;
    }

private:
    qreal m_radius;
    qreal m_sigma;
};

OpBlur::OpBlur(Process *parent) :
    Operator(OP_SECTION_EFFECTS, QT_TRANSLATE_NOOP("Operator", "Blur"), Operator::NonHDR, parent),
    m_radius(new OperatorParameterSlider("radius", "Radius", "Gaussian Blur Radius", Slider::Value, Slider::Logarithmic, Slider::Real, .1, 100, 1, .1, 1000, Slider::FilterPixels, this)),
    m_sigma(new OperatorParameterSlider("sigma", "Sigma", "Gaussian Blur Sigma", Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .5, 0, 1, Slider::FilterPercent, this))

{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_radius);
    addParameter(m_sigma);

}

OpBlur *OpBlur::newInstance()
{
    return new OpBlur(m_process);
}

OperatorWorker *OpBlur::newWorker()
{
    return new WorkerBlur(m_radius->value(),
                          m_radius->value()*m_sigma->value(),
                          m_thread, this);
}
