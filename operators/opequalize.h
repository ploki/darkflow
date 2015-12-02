#ifndef OPEQUALIZE_H
#define OPEQUALIZE_H
#include "operator.h"
#include <QObject>

class OpEqualize : public Operator
{
    Q_OBJECT
public:
    OpEqualize(Process *parent);
    OpEqualize *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPEQUALIZE_H
