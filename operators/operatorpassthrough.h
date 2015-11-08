#ifndef OperatorPassThrough_H
#define OperatorPassThrough_H

#include "operator.h"
#include <QObject>

class OperatorPassThrough : public Operator
{
    Q_OBJECT
public:
    OperatorPassThrough(Process *parent);
    ~OperatorPassThrough();
    OperatorPassThrough *newInstance();

    OperatorWorker *newWorker();
};

#endif // OperatorPassThrough_H
