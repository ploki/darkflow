#ifndef OPBLUR_H
#define OPBLUR_H

#include <QObject>
#include <operator.h>

class OperatorParameterSlider;

class OpBlur : public Operator
{
    Q_OBJECT
public:
    OpBlur(Process *parent);
    OpBlur *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_radius;
    OperatorParameterSlider *m_sigma;
};

#endif // OPBLUR_H
