#ifndef OPINVERT_H
#define OPINVERT_H

#include "operator.h"
#include <QObject>

class OpInvert : public Operator
{
    Q_OBJECT
public:
    OpInvert(Process *parent);

    OpInvert *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPINVERT_H
