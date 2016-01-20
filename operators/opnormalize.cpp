#include "opnormalize.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

class WorkerNormalize : public OperatorWorker {
public:
    WorkerNormalize(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo& photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().normalize();
        return newPhoto;
    }
};

OpNormalize::OpNormalize(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Normalize"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

}

OpNormalize *OpNormalize::newInstance()
{
    return new OpNormalize(m_process);
}

OperatorWorker *OpNormalize::newWorker()
{
    return new WorkerNormalize(m_thread, this);
}
