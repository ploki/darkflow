#include "opchannelmixer.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "channelmixer.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerChannelMixer : public OperatorWorker {
public:
    WorkerChannelMixer(qreal r, qreal g, qreal b, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_channelMixer(r,g,b)
    {}
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_channelMixer.applyOn(newPhoto);
        m_channelMixer.applyOnImage(newPhoto.curve());
        return newPhoto;
    }

private:
    ChannelMixer m_channelMixer;
};

OpChannelMixer::OpChannelMixer(Process *parent) :
    Operator(OP_SECTION_COLOR, "Channel Mixer", parent),
    m_r(new OperatorParameterSlider("red", "Red", "Channel Mixer Red Component", Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .2126, 0, 1, Slider::FilterPercent, this)),
    m_g(new OperatorParameterSlider("green", "Green", "Channel Mixer Green Component", Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .7152, 0, 1, Slider::FilterPercent, this)),
    m_b(new OperatorParameterSlider("blue", "Blue", "Channel Mixer Blue Component", Slider::Percent, Slider::Linear, Slider::Real, 0, 1, .0722, 0, 1, Slider::FilterPercent, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_r);
    addParameter(m_g);
    addParameter(m_b);
}

OpChannelMixer *OpChannelMixer::newInstance()
{
    return new OpChannelMixer(m_process);
}

OperatorWorker *OpChannelMixer::newWorker()
{
    return new WorkerChannelMixer(m_r->value(), m_g->value(), m_b->value(), m_thread, this);
}
