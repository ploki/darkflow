#ifndef OPERATOREXNIHILO_H
#define OPERATOREXNIHILO_H

#include "operator.h"
#include <QObject>

class Process;

class OperatorExNihilo : public Operator
{
    Q_OBJECT
public:
    OperatorExNihilo(Process *parent);
    ~OperatorExNihilo();
    OperatorExNihilo *newInstance();
    QString getClassIdentifier();


    OperatorWorker* newWorker();

};

#endif // OPERATOREXNIHILO_H
