#ifndef OPAIRYDISK_H
#define OPAIRYDISK_H

#include "operator.h"
#include <QObject>

class Process;
class OperatorParameterSlider;

class OpAiryDisk : public Operator
{
    Q_OBJECT
public:
    OpAiryDisk(Process *parent);
    ~OpAiryDisk();
    OpAiryDisk *newInstance();

    OperatorWorker* newWorker();
private:
    OperatorParameterSlider *m_diameter;
    OperatorParameterSlider *m_focal;
    OperatorParameterSlider *m_pixel_sz;
    OperatorParameterSlider *m_offset;
    OperatorParameterSlider *m_width;
    OperatorParameterSlider *m_precision;
};

#endif // OPAIRYDISK_H
