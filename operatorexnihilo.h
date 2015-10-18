#ifndef OPERATOREXNIHILO_H
#define OPERATOREXNIHILO_H

#include "operator.h"
#include <QObject>

class Process;
class Operator;

class OperatorExNihilo : public Operator
{
    Q_OBJECT
public:
    OperatorExNihilo(Process *parent);
    ~OperatorExNihilo();
    OperatorExNihilo *newInstance();
    QString getClassIdentifier();

public slots:
    void play();

protected:
    Image *process(const Image *);

};

#endif // OPERATOREXNIHILO_H
