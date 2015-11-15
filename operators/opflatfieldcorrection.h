#ifndef OPFLATFIELDCORRECTION_H
#define OPFLATFIELDCORRECTION_H

#include "operator.h"
#include <QObject>

class Process;

class OpFlatFieldCorrection : public Operator
{
    Q_OBJECT
public:
    OpFlatFieldCorrection(Process *parent);
    OpFlatFieldCorrection *newInstance();

    OperatorWorker *newWorker();

signals:

public slots:
};

#endif // OPFLATFIELDCORRECTION_H
