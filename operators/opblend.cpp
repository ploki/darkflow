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
    Operator(OP_SECTION_BLEND, "Blend", parent),
    m_mode1(new OperatorParameterDropDown("mode1", "A « B", BlendModeStr[Multiply], this)),
    m_mode2(new OperatorParameterDropDown("mode2", "AB « C", BlendModeStr[Multiply], this)),
    m_mode1Value(Multiply),
    m_mode2Value(Multiply)
{
    m_mode1->addOption(BlendModeStr[Multiply], this, SLOT(mode1Multiply()));
    m_mode1->addOption(BlendModeStr[Screen], this, SLOT(mode1Screen()));
    m_mode1->addOption(BlendModeStr[Overlay], this, SLOT(mode1Overlay()));
    m_mode1->addOption(BlendModeStr[HardLight], this, SLOT(mode1HardLight()));
    m_mode1->addOption(BlendModeStr[SoftLight], this, SLOT(mode1SoftLight()));
    m_mode1->addOption(BlendModeStr[DivideBrighten], this, SLOT(mode1Divide()));
    m_mode1->addOption(BlendModeStr[Divide], this, SLOT(mode1Divide()));
    m_mode1->addOption(BlendModeStr[DivideDarken], this, SLOT(mode1Divide()));
    m_mode1->addOption(BlendModeStr[Addition], this, SLOT(mode1Addition()));
    m_mode1->addOption(BlendModeStr[Subtract], this, SLOT(mode1Subtract()));
    m_mode1->addOption(BlendModeStr[Difference], this, SLOT(mode1Difference()));
    m_mode1->addOption(BlendModeStr[DarkenOnly], this, SLOT(mode1DarkenOnly()));
    m_mode1->addOption(BlendModeStr[LightenOnly], this, SLOT(mode1LightenOnly()));

    m_mode2->addOption(BlendModeStr[Multiply], this, SLOT(mode2Multiply()));
    m_mode2->addOption(BlendModeStr[Screen], this, SLOT(mode2Screen()));
    m_mode2->addOption(BlendModeStr[Overlay], this, SLOT(mode2Overlay()));
    m_mode2->addOption(BlendModeStr[HardLight], this, SLOT(mode2HardLight()));
    m_mode2->addOption(BlendModeStr[SoftLight], this, SLOT(mode2SoftLight()));
    m_mode2->addOption(BlendModeStr[DivideBrighten], this, SLOT(mode2Divide()));
    m_mode2->addOption(BlendModeStr[Divide], this, SLOT(mode2Divide()));
    m_mode2->addOption(BlendModeStr[DivideDarken], this, SLOT(mode2Divide()));
    m_mode2->addOption(BlendModeStr[Addition], this, SLOT(mode2Addition()));
    m_mode2->addOption(BlendModeStr[Subtract], this, SLOT(mode2Subtract()));
    m_mode2->addOption(BlendModeStr[Difference], this, SLOT(mode2Difference()));
    m_mode2->addOption(BlendModeStr[DarkenOnly], this, SLOT(mode2DarkenOnly()));
    m_mode2->addOption(BlendModeStr[LightenOnly], this, SLOT(mode2LightenOnly()));

    addParameter(m_mode1);
    addParameter(m_mode2);

    addInput(new OperatorInput("Layer A (top)","Layer A",OperatorInput::Set, this));
    addInput(new OperatorInput("Layer B (Middle)","Mayer B",OperatorInput::Set, this));
    addInput(new OperatorInput("Layer C (Bottom)","Layer C",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Blend","Blend",this));
    addOutput(new OperatorOutput("Overflow","Overflow",this));
    addOutput(new OperatorOutput("Underflow","Underflow",this));

}

OpBlend *OpBlend::newInstance()
{
    return new OpBlend(m_process);
}

OperatorWorker *OpBlend::newWorker()
{
    qDebug("mode1: %d, mode2: %d", m_mode1Value, m_mode2Value);
    return new WorkerBlend(m_mode1Value, m_mode2Value, m_thread, this);
}

void OpBlend::mode1Multiply()
{
    m_mode1Value = Multiply;
    setOutOfDate();
}

void OpBlend::mode1Screen()
{
    m_mode1Value = Screen;
    setOutOfDate();
}

void OpBlend::mode1Overlay()
{
    m_mode1Value = Overlay;
    setOutOfDate();
}

void OpBlend::mode1HardLight()
{
    m_mode1Value = HardLight;
    setOutOfDate();
}

void OpBlend::mode1SoftLight()
{
    m_mode1Value = SoftLight;
    setOutOfDate();
}

void OpBlend::mode1DivideBrighten()
{
    m_mode1Value = DivideBrighten;
    setOutOfDate();
}

void OpBlend::mode1Divide()
{
    m_mode1Value = Divide;
    setOutOfDate();
}

void OpBlend::mode1DivideDarken()
{
    m_mode1Value = DivideDarken;
    setOutOfDate();
}

void OpBlend::mode1Addition()
{
    m_mode1Value = Addition;
    setOutOfDate();
}

void OpBlend::mode1Subtract()
{
    m_mode1Value = Subtract;
    setOutOfDate();
}

void OpBlend::mode1Difference()
{
    m_mode1Value = Difference;
    setOutOfDate();
}

void OpBlend::mode1DarkenOnly()
{
    m_mode1Value = DarkenOnly;
    setOutOfDate();
}

void OpBlend::mode1LightenOnly()
{
    m_mode1Value = LightenOnly;
    setOutOfDate();
}

void OpBlend::mode2Multiply()
{
    m_mode2Value = Multiply;
    setOutOfDate();
}

void OpBlend::mode2Screen()
{
    m_mode2Value = Screen;
    setOutOfDate();
}

void OpBlend::mode2Overlay()
{
    m_mode2Value = Overlay;
    setOutOfDate();
}

void OpBlend::mode2HardLight()
{
    m_mode2Value = HardLight;
    setOutOfDate();
}

void OpBlend::mode2SoftLight()
{
    m_mode2Value = SoftLight;
    setOutOfDate();
}

void OpBlend::mode2DivideBrighten()
{
    m_mode2Value = DivideBrighten;
    setOutOfDate();
}

void OpBlend::mode2Divide()
{
    m_mode2Value = Divide;
    setOutOfDate();
}

void OpBlend::mode2DivideDarken()
{
    m_mode2Value = DivideDarken;
    setOutOfDate();
}

void OpBlend::mode2Addition()
{
    m_mode2Value = Addition;
    setOutOfDate();
}

void OpBlend::mode2Subtract()
{
    m_mode2Value = Subtract;
    setOutOfDate();
}

void OpBlend::mode2Difference()
{
    m_mode2Value = Difference;
    setOutOfDate();
}

void OpBlend::mode2DarkenOnly()
{
    m_mode2Value = DarkenOnly;
    setOutOfDate();
}

void OpBlend::mode2LightenOnly()
{
    m_mode2Value = LightenOnly;
    setOutOfDate();
}
