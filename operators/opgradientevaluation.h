#ifndef OPGRADIENTEVALUATION_H
#define OPGRADIENTEVALUATION_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpGradientEvaluation : public Operator
{
    Q_OBJECT
public:
    OpGradientEvaluation(Process *parent);
    OpGradientEvaluation *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_radius;
    OperatorParameterSlider *m_altitude;
    OperatorParameterSlider *m_pow;
};

#endif // OPGRADIENTEVALUATION_H
