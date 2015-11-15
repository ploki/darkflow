#ifndef OPSUBTRACT_H
#define OPSUBTRACT_H

#include "operator.h"
#include <QObject>

class OpSubtract : public Operator
{
    Q_OBJECT
public:
    OpSubtract(Process *parent);

    OpSubtract *newInstance();
    OperatorWorker *newWorker();

signals:

public slots:
};

#endif // OPSUBTRACT_H
