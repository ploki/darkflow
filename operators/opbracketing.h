#ifndef OPBRACKETING_H
#define OPBRACKETING_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpBracketing : public Operator
{
    Q_OBJECT
public:
    OpBracketing(Process *parent);

    OpBracketing *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_compensation;
    OperatorParameterSlider *m_high;
    OperatorParameterSlider *m_low;
};

#endif // OPBRACKETING_H
