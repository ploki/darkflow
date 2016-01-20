#include "opflip.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

class WorkerFlip : public OperatorWorker {
public:
    WorkerFlip(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo& photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().flip();
        return newPhoto;
    }
};

OpFlip::OpFlip(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, QT_TRANSLATE_NOOP("Operator", "Flip"), Operator::All, parent)
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

}

OpFlip *OpFlip::newInstance()
{
    return new OpFlip(m_process);
}

OperatorWorker *OpFlip::newWorker()
{
    return new WorkerFlip(m_thread, this);
}
