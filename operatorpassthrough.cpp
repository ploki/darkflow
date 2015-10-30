#include "operatorworker.h"
#include "operatorpassthrough.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "image.h"


class PassThrough : public OperatorWorker {
public:
    PassThrough(QThread *thread, Operator *op) :
    OperatorWorker(thread, op) {}

    Image *process(const Image *image) {
        return new Image(image->getFilename(), false);
    }
};



OperatorPassThrough::OperatorPassThrough(Process *parent) :
    Operator(parent)
{
    m_inputs.push_back(new OperatorInput("Images set 1","Images set # one",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Images set 2","Images set # two",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Images set 3","Images set # three",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("merge", "merge", this));
}

OperatorPassThrough::~OperatorPassThrough()
{
   // qWarning((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}


OperatorPassThrough *OperatorPassThrough::newInstance()
{
    return new OperatorPassThrough(m_process);
}

QString OperatorPassThrough::getClassIdentifier()
{
    return "Pass Through";
}

OperatorWorker *OperatorPassThrough::newWorker()
{
    qWarning("new PassThrough");
    return new PassThrough(m_thread, this);
}
