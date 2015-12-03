#ifndef OPGAUSSIANBLUR_H
#define OPGAUSSIANBLUR_H

#include <QObject>
#include <operator.h>

class OperatorParameterSlider;

class OpGaussianBlur : public Operator
{
    Q_OBJECT
public:
    OpGaussianBlur(Process *parent);
    OpGaussianBlur *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_radius;
    OperatorParameterSlider *m_sigma;
};

#endif // OPGAUSSIANBLUR_H
