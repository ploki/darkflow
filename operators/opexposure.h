#ifndef OPEXPOSURE_H
#define OPEXPOSURE_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpExposure : public Operator
{
    Q_OBJECT
public:
    OpExposure(Process *parent);
    OpExposure *newInstance();
    OperatorWorker *newWorker();

signals:

public slots:
private:
    OperatorParameterSlider *m_value;
};

#endif // OPEXPOSURE_H
