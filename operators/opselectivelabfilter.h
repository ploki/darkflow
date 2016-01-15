#ifndef OPSELECTIVELABFILTER_H
#define OPSELECTIVELABFILTER_H

#include "operator.h"

class OperatorParameterSlider;
class OperatorParameterDropDown;
class OperatorParameterSelectiveLab;

class OpSelectiveLabFilter : public Operator
{
    Q_OBJECT
public:
    typedef enum {
        Inside,
        Outside
    } Selection;
    OpSelectiveLabFilter(Process *parent);

    OpSelectiveLabFilter *newInstance();
    OperatorWorker *newWorker();

    Algorithm *getAlgorithm() const;
    void releaseAlgorithm(Algorithm *algo) const;

public slots:
    void exposureSelection(int v);

private:
    OperatorParameterSelectiveLab *m_selectiveLab;
    OperatorParameterSlider *m_saturation;
    OperatorParameterSlider *m_exposure;
    OperatorParameterDropDown *m_exposureSelection;
    Selection m_exposureSelectionValue;
};

#endif // OPSELECTIVELABFILTER_H
