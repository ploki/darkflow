#include "opdeconvolution.h"
#include "workerdeconvolution.h"
#include "operatorinput.h"
#include "operatoroutput.h"


OpDeconvolution::OpDeconvolution(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Deconvolution", parent)
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Kernel","Kernel",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Negative images", "Negative Images", this));
}

OpDeconvolution *OpDeconvolution::newInstance()
{
    return new OpDeconvolution(m_process);
}

OperatorWorker *OpDeconvolution::newWorker()
{
    return new WorkerDeconvolution(m_thread, this);
}
