#include "opadaptivethreshold.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>



using Magick::Quantum;

class WorkerAdaptiveThreshold : public OperatorWorker {
public:
    WorkerAdaptiveThreshold(int width, int height, quantum_t offset, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_width(width),
        m_height(height),
        m_offset(offset)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().adaptiveThreshold(m_width, m_height, m_offset);
        return newPhoto;
    }
private:
    int m_width;
    int m_height;
    quantum_t m_offset;
};

OpAdaptiveThreshold::OpAdaptiveThreshold(Process *parent) :
    Operator(OP_SECTION_COLOR, "Adaptive Threshold", parent),
    m_width(new OperatorParameterSlider("width", "Width", "Adaptive Threshold Width", Slider::Value, Slider::Linear, Slider::Integer, 1, 25, 4, 1, 1000, Slider::FilterNothing, this)),
    m_height(new OperatorParameterSlider("height", "Height", "Adaptive Threshold Height", Slider::Value, Slider::Linear, Slider::Integer, 1, 25, 4, 1, 1000, Slider::FilterNothing, this)),
    m_offset(new OperatorParameterSlider("offset", "Offset", "Adaptive Threshold Offset", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1, 1<<16, Slider::FilterExposureFromOne, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_width);
    addParameter(m_height);
    addParameter(m_offset);
}

OpAdaptiveThreshold *OpAdaptiveThreshold::newInstance()
{
    return new OpAdaptiveThreshold(m_process);
}

OperatorWorker *OpAdaptiveThreshold::newWorker()
{
    return new WorkerAdaptiveThreshold(round(m_width->value()),
                                       round(m_height->value()),
                                       (m_offset->value()-1.),
                                       m_thread, this);
}
