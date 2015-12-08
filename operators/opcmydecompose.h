#ifndef OPCMYDECOMPOSE_H
#define OPCMYDECOMPOSE_H

#include "operator.h"
#include <QObject>

class OpCMYDecompose : public Operator
{
    Q_OBJECT
public:
    OpCMYDecompose(Process *parent);
    OpCMYDecompose *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPCMYDECOMPOSE_H
