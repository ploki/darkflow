#include "opdespeckle.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

class WorkerDespeckle : public OperatorWorker {
public:
    WorkerDespeckle(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo& photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().despeckle();
        return newPhoto;
    }
};

OpDespeckle::OpDespeckle(Process *parent) :
    Operator(OP_SECTION_COSMETIC, QT_TRANSLATE_NOOP("Operator", "Despeckle"), Operator::NonHDR, parent)
{
    addInput(new OperatorInput(tr("Images"),OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

}

OpDespeckle *OpDespeckle::newInstance()
{
    return new OpDespeckle(m_process);
}

OperatorWorker *OpDespeckle::newWorker()
{
    return new WorkerDespeckle(m_thread, this);
}
