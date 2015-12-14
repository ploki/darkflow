#ifndef OPNORMALIZE_H
#define OPNORMALIZE_H

#include "operator.h"
#include <QObject>

class OpNormalize : public Operator
{
    Q_OBJECT
public:
    OpNormalize(Process *parent);
    OpNormalize *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPNORMALIZE_H
