#ifndef WORKERBLEND_H
#define WORKERBLEND_H

#include "operatorworker.h"
#include "opblend.h"

class WorkerBlend : public OperatorWorker {
public:
    WorkerBlend(OpBlend::BlendMode mode1,
                OpBlend::BlendMode mode2,
                bool outputHDR,
                QThread *thread,
                Operator *op);
    Photo process(const Photo &, int, int);
    void play();

private:
    OpBlend::BlendMode m_mode1;
    OpBlend::BlendMode m_mode2;
    bool m_outputHDR;

};

#endif // WORKERBLEND_H
