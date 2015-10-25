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
    OperatorParameterDropDown *m_debayer;
    OperatorParameterDropDown *m_whiteBalance;

    Debayer m_debayerValue;
    WhiteBalance m_whiteBalanceValue;

};

#endif // OPERATORLOADRAW_H
