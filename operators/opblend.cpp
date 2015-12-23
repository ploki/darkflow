#include "opblend.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerblend.h"

static const char *BlendModeStr[] = {
    "Multiply", "Screen", "Overlay", "Hard Light", "Soft Light",
    "Divide Brighten", "Divide", "Divide Darken",
    "Addition", "Subtract", "Difference",
    "Darken Only", "Lighten Only"
};


OpBlend::OpBlend(Process *parent) :
    Operator(OP_SECTION_BLEND, "Blend", Operator::All, parent),
    m_mode1(new OperatorParameterDropDown("mode1", "A « B", this, SLOT(selectMode1(int)))),
    m_mode2(new OperatorParameterDropDown("mode2", "AB « C", this, SLOT(selectMode2(int)))),
    m_mode1Value(Multiply),
    m_mode2Value(Multiply),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", "Output HDR", this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    registerOptions(m_mode1);
    registerOptions(m_mode2);
    addParameter(m_mode1);
    addParameter(m_mode2);

    m_outputHDR->addOption("No", false, true);
    m_outputHDR->addOption("Yes", true);

    addInput(new OperatorInput("Layer A (top)","Layer A",OperatorInput::Set, this));
    addInput(new OperatorInput("Layer B (Middle)","Mayer B",OperatorInput::Set, this));
    addInput(new OperatorInput("Layer C (Bottom)","Layer C",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Blend","Blend",this));
    addOutput(new OperatorOutput("Overflow","Overflow",this));
    addOutput(new OperatorOutput("Underflow","Underflow",this));
    addParameter(m_outputHDR);
}

void OpBlend::registerOptions(OperatorParameterDropDown *mode)
{
    mode->addOption(BlendModeStr[Multiply], Multiply, true);
    mode->addOption(BlendModeStr[Screen], Screen);
    mode->addOption(BlendModeStr[Overlay], Overlay);
    mode->addOption(BlendModeStr[HardLight], HardLight);
    mode->addOption(BlendModeStr[SoftLight], SoftLight);
    mode->addOption(BlendModeStr[DivideBrighten], DivideBrighten);
    mode->addOption(BlendModeStr[Divide], Divide);
    mode->addOption(BlendModeStr[DivideDarken], DivideDarken);
    mode->addOption(BlendModeStr[Addition], Addition);
    mode->addOption(BlendModeStr[Subtract], Subtract);
    mode->addOption(BlendModeStr[Difference], Difference);
    mode->addOption(BlendModeStr[DarkenOnly], DarkenOnly);
    mode->addOption(BlendModeStr[LightenOnly], LightenOnly);
}

OpBlend *OpBlend::newInstance()
{
    return new OpBlend(m_process);
}

OperatorWorker *OpBlend::newWorker()
{
    dflDebug("mode1: %d, mode2: %d", m_mode1Value, m_mode2Value);
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

