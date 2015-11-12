#ifndef OPMODULATE_H
#define OPMODULATE_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;

class OpModulate : public Operator
{
    Q_OBJECT
public:
    OpModulate(Process *parent);
    OpModulate *newInstance();
    OperatorWorker *newWorker();

signals:

public slots:
private:
    OperatorParameterSlider *m_hue;
    OperatorParameterSlider *m_saturation;
    OperatorParameterSlider *m_value;

};

#endif // OPMODULATE_H
