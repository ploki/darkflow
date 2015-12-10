#ifndef OPSCALE_H
#define OPSCALE_H

#include <QObject>
#include "operator.h"

class OperatorParameterSlider;
class OperatorParameterDropDown;

class OpScale : public Operator
{
    Q_OBJECT
public:
    typedef enum {
        Resize,
        Sample,
        Scale,
    } Function;
    typedef enum {
        ToSpecified,
        ToSmallestWidth,
        ToSmallestHeight,
        ToLargestWidth,
        ToLargestHeight,
    } ResizeTo;
    typedef Magick::FilterTypes ResizeAlgorithm;
    OpScale(Process *parent);
    OpScale *newInstance();
    OperatorWorker *newWorker();

private slots:
    void selectResizeTo(int v);
    void selectAlgorithm(int v);
    void selectFunction(int v);


private:
    OperatorParameterDropDown *m_function;
    Function m_functionValue;
    OperatorParameterDropDown *m_algorithm;
    int m_algorithmValue;
    OperatorParameterDropDown *m_to;
    ResizeTo m_toValue;
    OperatorParameterSlider *m_scale;
};

#endif // OPSCALE_H
