#ifndef OPFLOP_H
#define OPFLOP_H

#include "operator.h"
#include <QObject>

class OpFlop : public Operator
{
    Q_OBJECT
public:
    OpFlop(Process *parent);
    OpFlop *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPFLOP_H
