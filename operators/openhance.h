#ifndef OPENHANCE_H
#define OPENHANCE_H

#include "operator.h"
#include <QObject>

class OpEnhance : public Operator
{
    Q_OBJECT
public:
    OpEnhance(Process *parent);
    OpEnhance *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPENHANCE_H
