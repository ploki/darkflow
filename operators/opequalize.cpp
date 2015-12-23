#include "opequalize.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"

#include <Magick++.h>

class WorkerEqualize : public OperatorWorker {
public:
    WorkerEqualize(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {
    }
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        Magick::Image& image = newPhoto.image();
        image.modifyImage();
        image.equalize();
        return newPhoto;
    }
};

OpEqualize::OpEqualize(Process *parent) :
    Operator(OP_SECTION_COLOR, "Equalize", Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
}

OpEqualize *OpEqualize::newInstance()
{
    return new OpEqualize(m_process);
}

OperatorWorker *OpEqualize::newWorker()
{
    return new WorkerEqualize(m_thread, this);
}
