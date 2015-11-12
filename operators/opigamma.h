#ifndef OPIGAMMA_H
#define OPIGAMMA_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;
class OperatorParameterDropDown;

class OpIGamma : public Operator
{
    Q_OBJECT
public:
    OpIGamma(Process *parent);

    OpIGamma *newInstance();
    OperatorWorker *newWorker();

signals:

public slots:
    void revertYes();
    void revertNo();

private:
    OperatorParameterSlider *m_gamma;
    OperatorParameterSlider *m_dynamicRange;
    bool m_revert;
    OperatorParameterDropDown *m_revertDialog;

};

#endif // OPIGAMMA_H
