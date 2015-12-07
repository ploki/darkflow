#ifndef OPDEBAYER_H
#define OPDEBAYER_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;

class OpDebayer : public Operator
{
    Q_OBJECT
public:
    typedef enum {
        NoDebayer,
        HalfSize,
        Low,
        VNG,
        PPG,
        AHD,
    } Debayer;
    OpDebayer(Process *parent);
    OpDebayer *newInstance();
    OperatorWorker *newWorker();

public slots:
    void setDebayerNone();
    void setDebayerHalfSize();
    void setDebayerLow();
    void setDebayerVNG();
    void setDebayerPPG();
    void setDebayerAHD();
private:
    OperatorParameterDropDown *m_debayer;
    Debayer m_debayerValue;
};

#endif // OPDEBAYER_H
