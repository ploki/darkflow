#ifndef OPTHRESHOLD_H
#define OPTHRESHOLD_H

#include <QObject>
#include <operator.h>

class OperatorParameterSlider;

class OpThreshold : public Operator
{
    Q_OBJECT
public:
    OpThreshold(Process *parent);
    OpThreshold *newInstance();
    OperatorWorker *newWorker();

private:
    OperatorParameterSlider *m_high;
    OperatorParameterSlider *m_low;
};

#endif // OPTHRESHOLD_H
