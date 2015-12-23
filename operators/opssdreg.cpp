#include "opssdreg.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerssdreg.h"


OpSsdReg::OpSsdReg(Process *parent) :
    Operator(OP_SECTION_REGISTRATION, "SsdReg", Operator::All, parent)
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
}

OpSsdReg *OpSsdReg::newInstance()
{
return new OpSsdReg(m_process);
}

OperatorWorker *OpSsdReg::newWorker()
{
    return new WorkerSsdReg(m_thread, this);
}
