#ifndef OPDESATURATESHADOWS_H
#define OPDESATURATESHADOWS_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpDesaturateShadows : public Operator
{
    Q_OBJECT
public:
    OpDesaturateShadows(Process *parent);
    OpDesaturateShadows *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_highlightLimit;
    OperatorParameterSlider *m_range;
    OperatorParameterSlider *m_saturation;

};

#endif // OPDESATURATESHADOWS_H
