#ifndef OPLIMEREG_H
#define OPLIMEREG_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpLimereg : public Operator
{
    Q_OBJECT
public:
    OpLimereg(Process *parent);
    OpLimereg *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_maxRotation;
    OperatorParameterSlider *m_maxTranslationPercent;
};

#endif // OPLIMEREG_H
