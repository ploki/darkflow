#include "ophotpixels.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "hotpixels.h"
#include "ports.h"

class WorkerHotPixels : public OperatorWorker {
public:
    WorkerHotPixels(qreal delta, bool aggressive, bool naive, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_hotPixels(delta,aggressive, naive)
    {
    }
    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        m_hotPixels.applyOn(newPhoto);
        return newPhoto;
    }

private:
    HotPixels m_hotPixels;
};

OpHotPixels::OpHotPixels(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Hot Pixels", Operator::NonHDR, parent),
    m_delta(new OperatorParameterSlider("delta", "Delta", "Hot Pixels Delta", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, M_SQRT2l, 1, 1<<16, Slider::FilterExposureFromOne, this)),
    m_aggressive(new OperatorParameterDropDown("aggressive", "Aggressive", this, SLOT(selectAggressive(int)))),
    m_naive(new OperatorParameterDropDown("naive", "Naive", this, SLOT(selectNaive(int)))),
    m_aggressiveValue(true),
    m_naiveValue(false)
{
    m_aggressive->addOption("Yes", true, true);
    m_aggressive->addOption("No", false);
    m_naive->addOption("Yes", true);
    m_naive->addOption("No", false, true);

    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));

    addParameter(m_delta);
    addParameter(m_aggressive);
    addParameter(m_naive);
}

OpHotPixels *OpHotPixels::newInstance()
{
    return new OpHotPixels(m_process);
}

OperatorWorker *OpHotPixels::newWorker()
{
return new WorkerHotPixels(m_delta->value(),
                           m_aggressiveValue,
                           m_naiveValue,
                           m_thread, this);
}

void OpHotPixels::selectAggressive(int v)
{
    dflDebug("agg:%d, req:%d", m_aggressiveValue, v);
    if ( m_aggressiveValue != !!v) {
        m_aggressiveValue = !!v;
        setOutOfDate();
    }
}

void OpHotPixels::selectNaive(int v)
{
    dflDebug("naive:%d, req:%d", m_naiveValue, v);
    if ( m_naiveValue != !!v) {
        m_naiveValue = !!v;
        setOutOfDate();
    }
}
