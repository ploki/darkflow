#ifndef OPERATORROTATE_H
#define OPERATORROTATE_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;

class OpRotate : public Operator
{
    Q_OBJECT
public:
    OpRotate(Process *parent);
    ~OpRotate();

    OpRotate *newInstance();
    OperatorWorker *newWorker();
    qreal angle() const;

private slots:
    void setAngle(int v);

private:
    OperatorParameterDropDown *m_dropdown;
    qreal m_angle;
};

#endif // OPERATORROTATE_H
