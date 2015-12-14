#ifndef OPLEVEL_H
#define OPLEVEL_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpLevel : public Operator
{
    Q_OBJECT
public:
    OpLevel(Process *parent);
    OpLevel *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_blackPoint;
    OperatorParameterSlider *m_whitePoint;
    OperatorParameterSlider *m_gamma;
};

#endif // OPLEVEL_H
