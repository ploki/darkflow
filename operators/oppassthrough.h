#ifndef OperatorPassThrough_H
#define OperatorPassThrough_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpPassThrough : public Operator
{
    Q_OBJECT
public:
    OpPassThrough(Process *parent);
    OpPassThrough *newInstance();

    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_slider;
};

#endif // OperatorPassThrough_H
