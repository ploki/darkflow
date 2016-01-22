#include "opflop.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

class WorkerFlop : public OperatorWorker {
public:
    WorkerFlop(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo& photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().flop();
        return newPhoto;
    }
};

OpFlop::OpFlop(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, QT_TRANSLATE_NOOP("Operator", "Flop"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

}

OpFlop *OpFlop::newInstance()
{
    return new OpFlop(m_process);
}

OperatorWorker *OpFlop::newWorker()
{
    return new WorkerFlop(m_thread, this);
}
