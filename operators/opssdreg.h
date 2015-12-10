#ifndef OPSSDREG_H
#define OPSSDREG_H

#include "operator.h"
#include <QObject>

class OpSsdReg : public Operator
{
    Q_OBJECT
public:
    OpSsdReg(Process *parent);
    OpSsdReg *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPSSDREG_H
