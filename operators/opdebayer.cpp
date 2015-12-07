#include "opdebayer.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerdebayer.h"

static const char *DebayerStr[] = {
    "None", "Half Size", "Low", "VNG", "PPG", "AHD"
};

OpDebayer::OpDebayer(Process *parent) :
    Operator(OP_SECTION_COLOR, "Debayer", parent),
    m_debayer(new OperatorParameterDropDown("quality", "Quality", DebayerStr[NoDebayer], this)),
    m_debayerValue(AHD)
{
    m_debayer->addOption(DebayerStr[NoDebayer], this, SLOT(setDebayerNone()));
    m_debayer->addOption(DebayerStr[HalfSize], this, SLOT(setDebayerHalfSize()));
    m_debayer->addOption(DebayerStr[Low], this, SLOT(setDebayerLow()));
    m_debayer->addOption(DebayerStr[VNG], this, SLOT(setDebayerVNG()));
    m_debayer->addOption(DebayerStr[PPG], this, SLOT(setDebayerPPG()));
    m_debayer->addOption(DebayerStr[AHD], this, SLOT(setDebayerAHD()));
    addParameter(m_debayer);
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

}

void OpDebayer::setDebayerNone()
{
    if ( m_debayerValue != NoDebayer ) {
        m_debayerValue = NoDebayer;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerHalfSize()
{
    if ( m_debayerValue != HalfSize ) {
        m_debayerValue = HalfSize;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerLow()
{
    if ( m_debayerValue != Low ) {
        m_debayerValue = Low;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerVNG()
{
    if ( m_debayerValue != VNG ) {
        m_debayerValue = VNG;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerPPG()
{
    if ( m_debayerValue != PPG ) {
        m_debayerValue = PPG;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerAHD()
{
    if ( m_debayerValue != AHD ) {
        m_debayerValue = AHD;
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
