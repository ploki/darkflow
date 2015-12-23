#ifndef OPHDR_H
#define OPHDR_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;

class OpHDR : public Operator
{
    Q_OBJECT
public:
    OpHDR(Process *parent);

    OpHDR *newInstance();
    OperatorWorker *newWorker();

signals:

public slots:
    void revert(int b);

private:
    bool m_revert;
    OperatorParameterDropDown *m_revertDialog;

};

#endif // OPHDR_H
