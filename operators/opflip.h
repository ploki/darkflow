#ifndef OPFLIP_H
#define OPFLIP_H

#include "operator.h"
#include <QObject>

class OpFlip : public Operator
{
    Q_OBJECT
public:
    OpFlip(Process *parent);
    OpFlip *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPFLIP_H
