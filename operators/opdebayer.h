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
        Mask,
        HalfSize,
        Simple,
        Bilinear,
        HQLinear,
        /* DownSample is implemented by HalfSize */
        //DownSample,
        VNG,
        AHD
    } Debayer;
    OpDebayer(Process *parent);
    OpDebayer *newInstance();
    OperatorWorker *newWorker();

public slots:
    void setDebayer(int v);

private:
    OperatorParameterDropDown *m_debayer;
    Debayer m_debayerValue;
};

#endif // OPDEBAYER_H
