#ifndef OPERATOREXNIHILO_H
#define OPERATOREXNIHILO_H

#include "operator.h"
#include <QObject>

class Process;

class OpExNihilo : public Operator
{
    Q_OBJECT
public:
    OpExNihilo(Process *parent);
    ~OpExNihilo();
    OpExNihilo *newInstance();

    OperatorWorker* newWorker();

};

#endif // OPERATOREXNIHILO_H
