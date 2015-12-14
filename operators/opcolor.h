#ifndef OPCOLOR_H
#define OPCOLOR_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpColor : public Operator
{
    Q_OBJECT
public:
    OpColor(Process *parent);
    OpColor *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_r;
    OperatorParameterSlider *m_g;
    OperatorParameterSlider *m_b;
};

#endif // OPCOLOR_H
