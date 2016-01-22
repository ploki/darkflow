#include "opdesaturateshadows.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "desaturateshadows.h"
#include "photo.h"
#include <Magick++.h>

using Magick::Quantum;
class WorkerDeSha : public OperatorWorker {
public:
    WorkerDeSha(qreal highlightLimit, qreal range, qreal saturation, QThread *thread, OpDesaturateShadows *op) :
        OperatorWorker(thread, op),
        m_desha(highlightLimit,range, saturation)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_desha.applyOn(newPhoto);

        //nonsens
        //m_desha.applyOnImage(newPhoto.curve());
        return newPhoto;
    }

private:
    DesaturateShadows m_desha;
};

OpDesaturateShadows::OpDesaturateShadows(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Desaturate Shadows"), Operator::All, parent),
    m_highlightLimit(new OperatorParameterSlider("highlightLimit", tr("Higher limit"), tr("Desaturate Shadows Higher Limit"),Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<8),1, 1./(1<<5),1./QuantumRange,1, Slider::FilterExposureFromOne, this)),
    m_range(new OperatorParameterSlider("range", tr("Range"), tr("Desaturate Shadows Range"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<8, 1<<3, 1, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_saturation(new OperatorParameterSlider("saturation", tr("Saturation"), tr("Desaturate Shadows Saturation"), Slider::Percent, Slider::Linear, Slider::Integer, 0, 1, 0, 0, 1, Slider::FilterNothing, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

    addParameter(m_highlightLimit);
    addParameter(m_range);
    addParameter(m_saturation);
}

OpDesaturateShadows *OpDesaturateShadows::newInstance()
{
    return new OpDesaturateShadows(m_process);
}

OperatorWorker *OpDesaturateShadows::newWorker()
{
    return new WorkerDeSha(m_highlightLimit->value(),
                           m_range->value(),
                           m_saturation->value(),
                           m_thread, this);
}
