#include "opselectivelabfilter.h"
#include "operatorparameterselectivelab.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "selectivelabfilter.h"

class WorkerSelectiveLabFilter : public OperatorWorker {
public:
    WorkerSelectiveLabFilter(int hue, int coverage, int saturation, bool strict,
                             qreal exposure, bool insideSelection, QThread *thread, Operator *op)
        : OperatorWorker(thread, op),
          m_filter(new SelectiveLabFilter(hue, coverage, saturation, strict, exposure, insideSelection, false))
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_filter->applyOn(newPhoto);
        return newPhoto;
    }

private:
    SelectiveLabFilter *m_filter;
};

OpSelectiveLabFilter::OpSelectiveLabFilter(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Selective Lab Filter", Operator::All, parent),
    m_selectiveLab(new OperatorParameterSelectiveLab("labSelection", "Selection", "Selective Lab Filter", 0, 0, false, 35, true, false, this)),
    m_saturation(new OperatorParameterSlider("saturation", "Saturation", "Selective Lab Filter Saturation", Slider::Percent, Slider::Linear, Slider::Real, 0, 2, 1, 0, 10, Slider::FilterNothing, this)),
    m_exposure(new OperatorParameterSlider("exposure", "Exposure", "Selective Lab Filter Exposure", Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<8), 1<<8, 1, 1./(1<<16), 1<<16, Slider::FilterExposure, this)),
    m_exposureSelection(new OperatorParameterDropDown("exposureSelection", "Exposure zone", this, SLOT(exposureSelection(int)))),
    m_exposureSelectionValue(Inside)
{
    m_exposureSelection->addOption("Inside", Inside, true);
    m_exposureSelection->addOption("Outside", Outside);

    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_selectiveLab);
    addParameter(m_saturation);
    addParameter(m_exposure);
    addParameter(m_exposureSelection);
}

OpSelectiveLabFilter *OpSelectiveLabFilter::newInstance()
{
    return new OpSelectiveLabFilter(m_process);
}

OperatorWorker *OpSelectiveLabFilter::newWorker()
{
    return new WorkerSelectiveLabFilter(m_selectiveLab->hue(),
                                        m_selectiveLab->coverage(),
                                        m_saturation->value(),
                                        m_selectiveLab->strict(),
                                        m_exposure->value(),
                                        m_exposureSelectionValue==Inside, m_thread, this);
}

Algorithm *OpSelectiveLabFilter::getAlgorithm() const
{
    return new SelectiveLabFilter(m_selectiveLab->hue(),
                                  m_selectiveLab->coverage(),
                                  m_saturation->value(),
                                  m_selectiveLab->strict(),
                                  m_exposure->value(),
                                  m_exposureSelectionValue==Inside,
                                  false);
}

void OpSelectiveLabFilter::releaseAlgorithm(Algorithm *algo) const
{
    delete algo;
}

void OpSelectiveLabFilter::exposureSelection(int v)
{
    if ( m_exposureSelectionValue != v ) {
        m_exposureSelectionValue = Selection(v);
        setOutOfDate();
    }
}
