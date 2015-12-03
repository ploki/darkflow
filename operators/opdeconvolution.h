#ifndef OPDECONVOLUTION_H
#define OPDECONVOLUTION_H

#include "operator.h"
#include <QObject>

class OpDeconvolution : public Operator
{
    Q_OBJECT
public:
    OpDeconvolution(Process *parent);
    OpDeconvolution *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPDECONVOLUTION_H
