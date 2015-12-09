#ifndef OPWHITEBALANCE_H
#define OPWHITEBALANCE_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;
class OperatorParameterDropDown;

class OpWhiteBalance : public Operator
{
    Q_OBJECT
public:
    OpWhiteBalance(Process *parent);

    OpWhiteBalance *newInstance();
    OperatorWorker *newWorker();

public slots:
    void setSafe(int v);

private:
    OperatorParameterSlider *m_temperature;
    OperatorParameterSlider *m_tint;
    bool m_safe;
    OperatorParameterDropDown *m_safeDialog;
};

#endif // OPWHITEBALANCE_H
