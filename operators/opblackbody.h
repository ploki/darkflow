#ifndef OPBLACKBODY_H
#define OPBLACKBODY_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpBlackBody : public Operator
{
    Q_OBJECT
public:
    OpBlackBody(Process *parent);

    OpBlackBody *newInstance();
    OperatorWorker *newWorker();

private:
    OperatorParameterSlider *m_temperature;
    OperatorParameterSlider *m_tint;
    OperatorParameterSlider *m_value;
};

#endif // OPBLACKBODY_H
