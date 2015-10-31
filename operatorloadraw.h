#ifndef OPERATORLOADRAW_H
#define OPERATORLOADRAW_H

#include "operator.h"
#include <QObject>

class Process;
class OperatorParameterFilesCollection;
class OperatorParameterDropDown;
class OperatorLoadRaw : public Operator
{
    Q_OBJECT
public:
    OperatorLoadRaw(Process *parent);
    ~OperatorLoadRaw();
    OperatorLoadRaw *newInstance();
    QString getClassIdentifier();

    typedef enum {
        Linear,
        sRGB,
        IUT_BT_709
    } ColorSpace;
    typedef enum {
        NoDebayer,
        HalfSize,
        Low,
        VNG,
        PPG,
        AHD,
    } Debayer;
    typedef enum {
        NoWhiteBalance,
        Camera,
        Daylight,
    } WhiteBalance;

public slots:
    void setColorSpaceLinear();
    void setColorSpacesRGB();
    void setColorSpaceIUT_BT_709();
    void setDebayerNone();
    void setDebayerHalfSize();
    void setDebayerLow();
    void setDebayerVNG();
    void setDebayerPPG();
    void setDebayerAHD();

    void setWhiteBalanceNone();
    void setWhiteBalanceCamera();
    void setWhiteBalanceDaylight();

    void filesCollectionChanged();

    OperatorWorker *newWorker() { return NULL;}

protected:


private:
    OperatorParameterFilesCollection *m_filesCollection;
    OperatorParameterDropDown *m_colorSpace;
    OperatorParameterDropDown *m_debayer;
    OperatorParameterDropDown *m_whiteBalance;

    ColorSpace m_colorSpaceValue;
    Debayer m_debayerValue;
    WhiteBalance m_whiteBalanceValue;

};

#endif // OPERATORLOADRAW_H
