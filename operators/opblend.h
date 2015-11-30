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
        Divide,
        Addition,
        Subtract,
        Difference,
        DarkenOnly,
        LightenOnly
    }   BlendMode;

public slots:
    void mode1Multiply();
    void mode1Screen();
    void mode1Overlay();
    void mode1HardLight();
    void mode1SoftLight();
    void mode1Divide();
    void mode1Addition();
    void mode1Subtract();
    void mode1Difference();
    void mode1DarkenOnly();
    void mode1LightenOnly();

    void mode2Multiply();
    void mode2Screen();
    void mode2Overlay();
    void mode2HardLight();
    void mode2SoftLight();
    void mode2Divide();
    void mode2Addition();
    void mode2Subtract();
    void mode2Difference();
    void mode2DarkenOnly();
    void mode2LightenOnly();
private:
    OperatorParameterDropDown *m_mode1;
    OperatorParameterDropDown *m_mode2;
    BlendMode m_mode1Value;
    BlendMode m_mode2Value;
};

#endif // OPBLEND_H
