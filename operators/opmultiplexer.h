#ifndef OPMULTIPLEXER_H
#define OPMULTIPLEXER_H

#include "operator.h"
#include <QObject>

class OpMultiplexer : public Operator
{
    Q_OBJECT
public:
    OpMultiplexer(int ways, Process *parent);
    OpMultiplexer *newInstance();
    OperatorWorker *newWorker();
private:
    int m_ways;
};

#endif // OPMULTIPLEXER_H
