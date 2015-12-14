#ifndef WORKERGRADIENTEVALUATION_H
#define WORKERGRADIENTEVALUATION_H

#include "operatorworker.h"

class WorkerGradientEvaluation : public OperatorWorker
{
public:
    WorkerGradientEvaluation(qreal radius, qreal altitude, qreal pow_, QThread *thread, Operator *op);
    Photo process(const Photo &photo, int p, int c);

private:
    qreal m_radius;
    qreal m_altitude;
    qreal m_pow;
};

#endif // WORKERGRADIENTEVALUATION_H
