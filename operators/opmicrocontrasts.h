#ifndef OPMICROCONTRASTS_H
#define OPMICROCONTRASTS_H

#include "operator.h"
#include <QObject>

class OpMicroContrasts : public Operator
{
    Q_OBJECT
public:
    OpMicroContrasts(Process *parent);
    OpMicroContrasts *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPMICROCONTRASTS_H
