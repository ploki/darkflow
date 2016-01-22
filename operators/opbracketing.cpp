#include "opbracketing.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "photo.h"
#include "ports.h"
#include <Magick++.h>

class WorkerBracketing : public OperatorWorker {
public:
    WorkerBracketing(qreal compensation, qreal high, qreal low,
                     QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_compensation(compensation),
        m_high(high),
        m_low(low)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        newPhoto.setTag(TAG_HDR_COMP,QString::number(m_compensation));
        newPhoto.setTag(TAG_HDR_HIGH,QString::number(m_high));
        newPhoto.setTag(TAG_HDR_LOW,QString::number(m_low));
        return newPhoto;
    }

private:
    qreal m_compensation, m_high, m_low;
};

OpBracketing::OpBracketing(Process *parent) :
    Operator(OP_SECTION_BLEND, QT_TRANSLATE_NOOP("Operator", "Bracketing"), Operator::All, parent),
    m_compensation(new OperatorParameterSlider("compensation", tr("Compensation"), tr("Bracketing Exposure Compensation"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<16, 1, 1, 1<<16, Slider::FilterExposureFromOne, this)),
    m_high(new OperatorParameterSlider("high", tr("Limit High"), tr("Bracketing Limit High"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, M_SQRT1_2l, 1./(1<<16), 1<<16, Slider::FilterExposureFromOne, this)),
    m_low(new OperatorParameterSlider("low", tr("Limit Low"), tr("Bracketing Limit Low"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<16), 1, 1./(1<<8), 1./(1<<16), 1<<16, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_compensation);
    addParameter(m_high);
    addParameter(m_low);
}

OpBracketing *OpBracketing::newInstance()
{
    return new OpBracketing(m_process);
}

OperatorWorker *OpBracketing::newWorker()
{
    return new WorkerBracketing(m_compensation->value(),
                                m_high->value(),
                                m_low->value(),
                                m_thread, this);
}
