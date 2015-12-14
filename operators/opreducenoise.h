#ifndef OPREDUCENOISE_H
#define OPREDUCENOISE_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpReduceNoise : public Operator
{
    Q_OBJECT
public:
    OpReduceNoise(Process *parent);
    OpReduceNoise *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_order;
};

#endif // OPREDUCENOISE_H
