#ifndef OPLOADIMAGE_H
#define OPLOADIMAGE_H

#include <QObject>
#include "operator.h"

class OperatorParameterFilesCollection;
class OperatorParameterDropDown;

class OpLoadImage : public Operator
{
    Q_OBJECT
public:
    OpLoadImage(Process *parent);
    OpLoadImage *newInstance();
    OperatorWorker *newWorker();

    typedef enum {
        Linear,
        sRGB,
        IUT_BT_709
    } ColorSpace;


public slots:
    void setColorSpaceLinear();
    void setColorSpacesRGB();
    void setColorSpaceIUT_BT_709();
    void filesCollectionChanged();
private:
    OperatorParameterFilesCollection *m_filesCollection;
    OperatorParameterDropDown *m_colorSpace;
    ColorSpace m_colorSpaceValue;

};

#endif // OPLOADIMAGE_H
