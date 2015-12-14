#ifndef OPHOTPIXELS_H
#define OPHOTPIXELS_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;
class OperatorParameterDropDown;

class OpHotPixels : public Operator
{
    Q_OBJECT
public:
    OpHotPixels(Process *parent);
    OpHotPixels *newInstance();
    OperatorWorker *newWorker();
private slots:
    void selectAggressive(int v);
    void selectNaive(int v);
private:
    OperatorParameterSlider *m_delta;
    OperatorParameterDropDown *m_aggressive;
    OperatorParameterDropDown *m_naive;
    bool m_aggressiveValue;
    bool m_naiveValue;
};

#endif // OPHOTPIXELS_H
