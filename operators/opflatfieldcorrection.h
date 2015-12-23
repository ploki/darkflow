#ifndef OPFLATFIELDCORRECTION_H
#define OPFLATFIELDCORRECTION_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;

class OpFlatFieldCorrection : public Operator
{
    Q_OBJECT
public:
    OpFlatFieldCorrection(Process *parent);
    OpFlatFieldCorrection *newInstance();

    OperatorWorker *newWorker();

signals:

public slots:
    void setOutputHDR(int type);
private:
    OperatorParameterDropDown *m_outputHDR;
    bool m_outputHDRValue;

};

#endif // OPFLATFIELDCORRECTION_H
