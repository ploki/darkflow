#ifndef OPRGBCOMPOSE_H
#define OPRGBCOMPOSE_H

#include "operator.h"
#include <QObject>

class OpRGBCompose : public Operator
{
    Q_OBJECT
public:
    OpRGBCompose(Process *parent);
    OpRGBCompose *newInstance();
    OperatorWorker *newWorker();
};

#endif // OPRGBCOMPOSE_H
