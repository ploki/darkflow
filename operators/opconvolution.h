#ifndef OPCONVOLUTION_H
#define OPCONVOLUTION_H
#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpConvolution : public Operator
{
    Q_OBJECT
public:
    OpConvolution(Process *parent);
    OpConvolution *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_luminosity;
};
#endif // OPCONVOLUTION_H
