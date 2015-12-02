#ifndef OPRGBDECOMPOSE_H
#define OPRGBDECOMPOSE_H

#include "operator.h"
#include <QObject>

class OpRGBDecompose : public Operator
{
    Q_OBJECT
public:
    OpRGBDecompose(Process *parent);
    OpRGBDecompose *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPRGBDECOMPOSE_H
