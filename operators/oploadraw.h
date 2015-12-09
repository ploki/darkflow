#ifndef OPERATORLOADRAW_H
#define OPERATORLOADRAW_H

#include "operator.h"
#include <QObject>

class Process;
class OperatorParameterFilesCollection;
class OperatorParameterDropDown;
class OpLoadRaw : public Operator
{
    Q_OBJECT
public:
    OpLoadRaw(Process *parent);
    ~OpLoadRaw();
    OpLoadRaw *newInstance();

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
    void setColorSpace(int v);
    void setDebayer(int v);
    void setWhiteBalance(int v);

    void filesCollectionChanged();

    OperatorWorker *newWorker();

protected:


private:
    friend class WorkerLoadRaw;
    OperatorParameterFilesCollection *m_filesCollection;
    OperatorParameterDropDown *m_colorSpace;
    OperatorParameterDropDown *m_debayer;
    OperatorParameterDropDown *m_whiteBalance;

    ColorSpace m_colorSpaceValue;
    Debayer m_debayerValue;
    WhiteBalance m_whiteBalanceValue;

};

#endif // OPERATORLOADRAW_H
