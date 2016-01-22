#include "opdebayer.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerdebayer.h"

static const char *DebayerStr[] = {
    QT_TRANSLATE_NOOP("OpDebayer", "None"),
    QT_TRANSLATE_NOOP("OpDebayer", "Mask"),
    QT_TRANSLATE_NOOP("OpDebayer", "HalfSize"),
    QT_TRANSLATE_NOOP("OpDebayer", "Simple"),
    QT_TRANSLATE_NOOP("OpDebayer", "Bilinear"),
    QT_TRANSLATE_NOOP("OpDebayer", "HQ Linear"),
    //QT_TRANSLATE_NOOP("OpDebayer", "Down Sample"),
    QT_TRANSLATE_NOOP("OpDebayer", "VNG"),
    QT_TRANSLATE_NOOP("OpDebayer", "AHD")
};


OpDebayer::OpDebayer(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Debayer"), Operator::All, parent),
    m_debayer(new OperatorParameterDropDown("quality", tr("Quality"), this, SLOT(setDebayer(int)))),
    m_debayerValue(Bilinear)
{
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[NoDebayer]), NoDebayer);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Mask]), Mask);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[HalfSize]), HalfSize);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Simple]), Simple);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Bilinear]), Bilinear);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[HQLinear]), HQLinear, true);
    //m_debayer->addOption(DF_TR_AND_C(DebayerStr[DownSample]), DownSample);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[VNG]), VNG);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[AHD]), AHD);
    addParameter(m_debayer);
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

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
