#ifndef OPCMYCOMPOSE_H
#define OPCMYCOMPOSE_H

#include "operator.h"
#include <QObject>

class OpCMYCompose : public Operator
{
    Q_OBJECT
public:
    OpCMYCompose(Process *parent);
    OpCMYCompose *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPCMYCOMPOSE_H
