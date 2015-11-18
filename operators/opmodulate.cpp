#include "opmodulate.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>
#include "photo.h"

using Magick::Quantum;

class WorkerModulate : public OperatorWorker {
public:
    WorkerModulate(qreal hue, qreal saturation, qreal value, QThread *thread, OpModulate *op) :
        OperatorWorker(thread, op),
        m_hue(hue),
        m_saturation(saturation),
        m_value(value)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        Magick::Image& image = newPhoto.image();
        image.modifyImage();
        image.modulate(m_value, m_saturation, m_hue);
        newPhoto.curve().modulate(m_value, 100, 100);
        return newPhoto;
    }
private:
    qreal m_hue;
    qreal m_saturation;
    qreal m_value;
};


OpModulate::OpModulate(Process *parent) :
    Operator(OP_SECTION_CURVE, "Modulate", parent),
    m_hue(new OperatorParameterSlider("hue", "Hue", "Modulate Hue",Slider::Value, Slider::Linear, Slider::Integer, -180, 180, 0, -180, 180, Slider::FilterNothing, this)),
    m_saturation(new OperatorParameterSlider("saturation", "Saturation", "Modulate Saturation",Slider::Percent, Slider::Linear, Slider::Integer, 0, 2, 1, 0, 10, Slider::FilterNothing, this)),
    m_value(new OperatorParameterSlider("value", "Exposure", "Modulate Exposure",Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<8, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));

    m_parameters.push_back(m_hue);
    m_parameters.push_back(m_saturation);
    m_parameters.push_back(m_value);
}

OpModulate *OpModulate::newInstance()
{
    return new OpModulate(m_process);
}

OperatorWorker *OpModulate::newWorker()
{
    return new WorkerModulate(100+m_hue->value()/1.8,
                              m_saturation->value()*100.,
                              m_value->value()*100., m_thread, this);
}
