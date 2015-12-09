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
    m_debayer(new OperatorParameterDropDown("quality", "Quality", DebayerStr[Bilinear], this)),
    m_debayerValue(Bilinear)
{
    m_debayer->addOption(DebayerStr[NoDebayer], this, SLOT(setDebayerNone()));
    m_debayer->addOption(DebayerStr[Mask], this, SLOT(setDebayerMask()));
    m_debayer->addOption(DebayerStr[HalfSize], this, SLOT(setDebayerHalfSize()));
    m_debayer->addOption(DebayerStr[Simple], this, SLOT(setDebayerSimple()));
    m_debayer->addOption(DebayerStr[Bilinear], this, SLOT(setDebayerBilinear()));
    m_debayer->addOption(DebayerStr[HQLinear], this, SLOT(setDebayerHQLinear()));
    //m_debayer->addOption(DebayerStr[DownSample], this, SLOT(setDebayerDownSample()));
    m_debayer->addOption(DebayerStr[VNG], this, SLOT(setDebayerVNG()));
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

void OpDebayer::setDebayerMask()
{
    if ( m_debayerValue != Mask ) {
        m_debayerValue = Mask;
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

void OpDebayer::setDebayerSimple()
{
    if ( m_debayerValue != Simple ) {
        m_debayerValue = Simple;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerBilinear()
{
    if ( m_debayerValue != Bilinear ) {
        m_debayerValue = Bilinear;
        setOutOfDate();
    }
}

void OpDebayer::setDebayerHQLinear()
{
    if ( m_debayerValue != HQLinear ) {
        m_debayerValue = HQLinear;
        setOutOfDate();
    }
}

/*
void OpDebayer::setDebayerDownSample()
{
    if ( m_debayerValue != DownSample ) {
        m_debayerValue = DownSample;
        setOutOfDate();
    }
}
*/

void OpDebayer::setDebayerVNG()
{
    if ( m_debayerValue != VNG ) {
        m_debayerValue = VNG;
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
