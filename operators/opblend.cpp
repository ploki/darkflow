#include "opblend.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerblend.h"

static const char *BlendModeStr[] = {
    QT_TRANSLATE_NOOP("OpBlend", "Multiply"), QT_TRANSLATE_NOOP("OpBlend", "Screen"), QT_TRANSLATE_NOOP("OpBlend", "Overlay"), QT_TRANSLATE_NOOP("OpBlend", "Hard Light"),
    QT_TRANSLATE_NOOP("OpBlend", "Soft Light"), QT_TRANSLATE_NOOP("OpBlend", "Divide Brighten"), QT_TRANSLATE_NOOP("OpBlend", "Divide"), QT_TRANSLATE_NOOP("OpBlend", "Divide Darken"),
    QT_TRANSLATE_NOOP("OpBlend", "Addition"), QT_TRANSLATE_NOOP("OpBlend", "Subtract"), QT_TRANSLATE_NOOP("OpBlend", "Difference"),
    QT_TRANSLATE_NOOP("OpBlend", "Darken Only"), QT_TRANSLATE_NOOP("OpBlend", "Lighten Only")
};


OpBlend::OpBlend(Process *parent) :
    Operator(OP_SECTION_BLEND, QT_TRANSLATE_NOOP("Operator", "Blend"), Operator::All, parent),
    m_mode1(new OperatorParameterDropDown("mode1", tr("A « B"), this, SLOT(selectMode1(int)))),
    m_mode2(new OperatorParameterDropDown("mode2", tr("AB « C"), this, SLOT(selectMode2(int)))),
    m_mode1Value(Multiply),
    m_mode2Value(Multiply),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    registerOptions(m_mode1);
    registerOptions(m_mode2);
    addParameter(m_mode1);
    addParameter(m_mode2);

    m_outputHDR->addOption("No", false, true);
    m_outputHDR->addOption("Yes", true);

    addInput(new OperatorInput(tr("Layer A (top)"),tr("Layer A"),OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Layer B (Middle)"),tr("Layer B"),OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Layer C (Bottom)"),tr("Layer C"),OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Blend"),tr("Blend"),this));
    addOutput(new OperatorOutput(tr("Overflow"),tr("Overflow"),this));
    addOutput(new OperatorOutput(tr("Underflow"),tr("Underflow"),this));
    addParameter(m_outputHDR);
}

void OpBlend::registerOptions(OperatorParameterDropDown *mode)
{
    mode->addOption(tr(BlendModeStr[Multiply]), Multiply, true);
    mode->addOption(tr(BlendModeStr[Screen]), Screen);
    mode->addOption(tr(BlendModeStr[Overlay]), Overlay);
    mode->addOption(tr(BlendModeStr[HardLight]), HardLight);
    mode->addOption(tr(BlendModeStr[SoftLight]), SoftLight);
    mode->addOption(tr(BlendModeStr[DivideBrighten]), DivideBrighten);
    mode->addOption(tr(BlendModeStr[Divide]), Divide);
    mode->addOption(tr(BlendModeStr[DivideDarken]), DivideDarken);
    mode->addOption(tr(BlendModeStr[Addition]), Addition);
    mode->addOption(tr(BlendModeStr[Subtract]), Subtract);
    mode->addOption(tr(BlendModeStr[Difference]), Difference);
    mode->addOption(tr(BlendModeStr[DarkenOnly]), DarkenOnly);
    mode->addOption(tr(BlendModeStr[LightenOnly]), LightenOnly);
}

OpBlend *OpBlend::newInstance()
{
    return new OpBlend(m_process);
}

OperatorWorker *OpBlend::newWorker()
{
    dflDebug(tr("mode1: %0, mode2: %1").arg(m_mode1Value).arg(m_mode2Value));
    return new WorkerBlend(m_mode1Value, m_mode2Value, m_outputHDRValue, m_thread, this);
}

void OpBlend::selectMode1(int v)
{
    if ( m_mode1Value != v ) {
        m_mode1Value = BlendMode(v);
        setOutOfDate();
    }
}

void OpBlend::selectMode2(int v)
{
    if ( m_mode2Value != v ) {
        m_mode2Value = BlendMode(v);
        setOutOfDate();
    }
}
void OpBlend::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}

