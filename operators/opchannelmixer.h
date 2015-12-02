#ifndef OPCHANNELMIXER_H
#define OPCHANNELMIXER_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpChannelMixer : public Operator
{
    Q_OBJECT
public:
    OpChannelMixer(Process *parent);
    OpChannelMixer *newInstance();
    OperatorWorker *newWorker();
private:
    OperatorParameterSlider *m_r;
    OperatorParameterSlider *m_g;
    OperatorParameterSlider *m_b;
};

#endif // OPCHANNELMIXER_H
