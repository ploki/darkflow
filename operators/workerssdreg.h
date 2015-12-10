#ifndef WORKERSSDREG_H
#define WORKERSSDREG_H

#include "operatorworker.h"

class WorkerSsdReg : public OperatorWorker
{
    Q_OBJECT
public:
    WorkerSsdReg(QThread *thread, Operator *op);
    Photo process(const Photo &photo, int, int);
    void play_analyseSources();
    bool play_onInput(int idx);
private:
    int m_refIdx;
};

#endif // WORKERSSDREG_H
