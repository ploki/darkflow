#ifndef OPBLEND_H
#define OPBLEND_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;

class OpBlend : public Operator
{
    Q_OBJECT
public:
    OpBlend(Process *parent);
    OpBlend *newInstance();
    OperatorWorker *newWorker();

    typedef enum {
        Multiply,
        Screen,
        Overlay,
        HardLight,
        SoftLight,
        DivideBrighten,
        Divide,
        DivideDarken,
        Addition,
        Subtract,
        Difference,
        DarkenOnly,
        LightenOnly
    }   BlendMode;

public slots:
    void selectMode1(int v);
    void selectMode2(int v);
private:
    OperatorParameterDropDown *m_mode1;
    OperatorParameterDropDown *m_mode2;
    BlendMode m_mode1Value;
    BlendMode m_mode2Value;

    void registerOptions(OperatorParameterDropDown *mode);
};

#endif // OPBLEND_H
