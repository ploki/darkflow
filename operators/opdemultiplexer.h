#ifndef OPDEMULTIPLEXER_H
#define OPDEMULTIPLEXER_H

#include "operator.h"
#include <QObject>

class OpDemultiplexer : public Operator
{
    Q_OBJECT
public:
    OpDemultiplexer(int ways, Process *parent);
    OpDemultiplexer *newInstance();
    OperatorWorker *newWorker();
private:
    int m_ways;
};

#endif // OPDEMULTIPLEXER_H
