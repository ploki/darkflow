#include "opinvert.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "invert.h"

class WorkerInvert : public OperatorWorker {
public:
    WorkerInvert(QThread *thread, OpInvert *op) :
        OperatorWorker(thread, op),
        m_invert()
    {}
    Photo process(const Photo& photo, int, int) {
        Photo newPhoto(photo);
        m_invert.applyOn(newPhoto);
        m_invert.applyOnImage(newPhoto.curve());
        return newPhoto;
    }
private:
    Invert m_invert;
};

OpInvert::OpInvert(Process *parent) :
    Operator(OP_SECTION_COLOR, "Invert", parent)
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Negative images", "Negative Images", this));

}

OpInvert *OpInvert::newInstance()
{
    return new OpInvert(m_process);
}

OperatorWorker *OpInvert::newWorker()
{
    return new WorkerInvert(m_thread, this);
}
