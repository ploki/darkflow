#ifndef OPROLL_H
#define OPROLL_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpRoll : public Operator
{
    Q_OBJECT
public:
    OpRoll(Process *parent);
    OpRoll *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_columns;
    OperatorParameterSlider *m_rows;
};

#endif // OPROLL_H
