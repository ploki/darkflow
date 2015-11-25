#ifndef OPCROP_H
#define OPCROP_H

#include "operator.h"
#include <QObject>

class OpCrop : public Operator
{
public:
    OpCrop(Process *parent);

    OpCrop *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPCROP_H
