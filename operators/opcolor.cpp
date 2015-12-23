#include "opcolor.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatorparameterslider.h"
#include "operatoroutput.h"
#include <Magick++.h>
using Magick::Quantum;

class WorkerColor : public OperatorWorker {
public:
    WorkerColor(qreal r, qreal g, qreal b, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_r(r),
        m_g(g),
        m_b(b)
    {}
    Photo process(const Photo &, int, int) { throw 0; }

    void play() {
        Photo photo;
        photo.setIdentity(m_operator->uuid());
        photo.createImage(1,1);
        quantum_t r = round((m_r-(1./(1<<16)))*(1<<16));
        quantum_t g = round((m_g-(1./(1<<16)))*(1<<16));
        quantum_t b = round((m_b-(1./(1<<16)))*(1<<16));
        photo.setTag(TAG_NAME, "Color");
        photo.image().pixelColor(0, 0, Magick::Color(r,g,b));
        outputPush(0, photo);
        emitSuccess();
    }

private:
    qreal m_r;
    qreal m_g;
    qreal m_b;
};
OpColor::OpColor(Process *parent) :
    Operator(OP_SECTION_COLOR, "Color", Operator::NA, parent),
    m_r(new OperatorParameterSlider("red", "Red", "Color Red Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1, 1./(1<<16), 1, Slider::FilterExposureFromOne, this)),
    m_g(new OperatorParameterSlider("green", "Green", "Color Green Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1, 1./(1<<16), 1, Slider::FilterExposureFromOne, this)),
    m_b(new OperatorParameterSlider("blue", "Blue", "Color Blue Component", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1, 1./(1<<16), 1, Slider::FilterExposureFromOne, this))
{
    addOutput(new OperatorOutput("Color", "Color", this));
    addParameter(m_r);
    addParameter(m_g);
    addParameter(m_b);
}

OpColor *OpColor::newInstance()
{
    return new OpColor(m_process);
}

OperatorWorker *OpColor::newWorker()
{
    return new WorkerColor(m_r->value(), m_g->value(), m_b->value(), m_thread, this);
}
