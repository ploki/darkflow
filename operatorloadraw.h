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
        RawColors,
        Camera,
        Daylight,
    } WhiteBalance;

    QStringList getCollection() const;

    QString getColorSpace() const;
    QString getDebayer() const;
    QString getWhiteBalance() const;

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
    void setWhiteBalanceRawColors();
    void setWhiteBalanceCamera();
    void setWhiteBalanceDaylight();

    void filesCollectionChanged();

    OperatorWorker *newWorker();

protected:


private:
    friend class RawConvert;
    OperatorParameterFilesCollection *m_filesCollection;
    OperatorParameterDropDown *m_colorSpace;
    OperatorParameterDropDown *m_debayer;
    OperatorParameterDropDown *m_whiteBalance;

    ColorSpace m_colorSpaceValue;
    Debayer m_debayerValue;
    WhiteBalance m_whiteBalanceValue;

};

#endif // OPERATORLOADRAW_H
