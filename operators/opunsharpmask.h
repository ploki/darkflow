#ifndef OPUNSHARPMASK_H
#define OPUNSHARPMASK_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpUnsharpMask : public Operator
{
    Q_OBJECT
public:
    OpUnsharpMask(Process *parent);
    OpUnsharpMask *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_radius;
    OperatorParameterSlider *m_sigma;
    OperatorParameterSlider *m_amount;
    OperatorParameterSlider *m_threshold;
};

#endif // OPUNSHARPMASK_H
