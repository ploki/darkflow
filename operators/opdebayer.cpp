#include "opdebayer.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerdebayer.h"

static const char *DebayerStr[] = {
    "None",
    "Mask",
    "HalfSize",
    "Simple",
    "Bilinear",
    "HQ Linear",
    //"Down Sample",
    "VNG",
    "AHD"
};


OpDebayer::OpDebayer(Process *parent) :
    Operator(OP_SECTION_COLOR, "Debayer", parent),
    m_debayer(new OperatorParameterDropDown("quality", "Quality", this, SLOT(setDebayer(int)))),
    m_debayerValue(Bilinear)
{
    m_debayer->addOption(DebayerStr[NoDebayer], NoDebayer);
    m_debayer->addOption(DebayerStr[Mask], Mask);
    m_debayer->addOption(DebayerStr[HalfSize], HalfSize);
    m_debayer->addOption(DebayerStr[Simple], Simple);
    m_debayer->addOption(DebayerStr[Bilinear], Bilinear);
    m_debayer->addOption(DebayerStr[HQLinear], HQLinear, true);
    //m_debayer->addOption(DebayerStr[DownSample], DownSample);
    m_debayer->addOption(DebayerStr[VNG], VNG);
    m_debayer->addOption(DebayerStr[AHD], AHD);
    addParameter(m_debayer);
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

}

void OpDebayer::setDebayer(int v)
{
    if ( m_debayerValue != v ) {
        m_debayerValue = Debayer(v);
        setOutOfDate();
    }
}

OpDebayer *OpDebayer::newInstance()
{
    return new OpDebayer(m_process);
}

OperatorWorker *OpDebayer::newWorker()
{
    return new WorkerDebayer(m_debayerValue, m_thread, this);
}
