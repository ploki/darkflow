#ifndef OPLEVELPERCENTILE_H
#define OPLEVELPERCENTILE_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpLevelPercentile : public Operator
{
    Q_OBJECT
public:
    OpLevelPercentile(Process *parent);
    OpLevelPercentile *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_blackPoint;
    OperatorParameterSlider *m_whitePoint;
    OperatorParameterSlider *m_gamma;
};

#endif // OPLEVELPERCENTILE_H
