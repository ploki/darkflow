#ifndef OPDESPECKLE_H
#define OPDESPECKLE_H

#include "operator.h"
#include <QObject>

class OpDespeckle : public Operator
{
    Q_OBJECT
public:
    OpDespeckle(Process *parent);
    OpDespeckle *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPDESPECKLE_H
