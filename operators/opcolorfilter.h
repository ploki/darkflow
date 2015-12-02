#ifndef OPCOLORFILTER_H
#define OPCOLORFILTER_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpColorFilter : public Operator
{
    Q_OBJECT
public:
    OpColorFilter(Process *parent);
    OpColorFilter *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_r;
    OperatorParameterSlider *m_g;
    OperatorParameterSlider *m_b;
};

#endif // OPCOLORFILTER_H
