#include "operatorworker.h"
#include "operatorpassthrough.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"

#include "operatorparameterslider.h"

class PassThrough : public OperatorWorker {
public:
    PassThrough(QThread *thread, Operator *op) :
    OperatorWorker(thread, op) {}

    Photo process(const Photo &photo, int, int) {
        return Photo(photo);
    }
};



OperatorPassThrough::OperatorPassThrough(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, "Pass Through", parent),
    m_slider(new OperatorParameterSlider("scale", "scale", "scale",
                                         Slider::ExposureValue, Slider::Logarithmic,
                                         Slider::Real,
                                         1., 1<<4,
                                         1.,
                                         1./65535, 65535,
                                         Slider::FilterAll,this))
{
    addInput(new OperatorInput("Images set 1","Images set # one",OperatorInput::Set, this));
    addInput(new OperatorInput("Images set 2","Images set # two",OperatorInput::Set, this));
    addInput(new OperatorInput("Images set 3","Images set # three",OperatorInput::Set, this));
    addOutput(new OperatorOutput("merge", "merge", this));

    addParameter(m_slider);
}

OperatorPassThrough::~OperatorPassThrough()
{
   // qDebug((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}


OperatorPassThrough *OperatorPassThrough::newInstance()
{
    return new OperatorPassThrough(m_process);
}

OperatorWorker *OperatorPassThrough::newWorker()
{
    return new PassThrough(m_thread, this);
}
