#ifndef OPADAPTIVETHRESHOLD_H
#define OPADAPTIVETHRESHOLD_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpAdaptiveThreshold : public Operator
{
    Q_OBJECT
public:
    OpAdaptiveThreshold(Process *parent);
    OpAdaptiveThreshold *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_width;
    OperatorParameterSlider *m_height;
    OperatorParameterSlider *m_offset;
};

#endif // OPADAPTIVETHRESHOLD_H
