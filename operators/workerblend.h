#ifndef WORKERBLEND_H
#define WORKERBLEND_H

#include "operatorworker.h"
#include "opblend.h"

class WorkerBlend : public OperatorWorker {
public:
    WorkerBlend(OpBlend::BlendMode mode1,
                OpBlend::BlendMode mode2,
                QThread *thread,
                Operator *op);
    Photo process(const Photo &, int, int);
    void play(QVector<QVector<Photo> > inputs, int n_outputs);

private:
    OpBlend::BlendMode m_mode1;
    OpBlend::BlendMode m_mode2;
};

#endif // WORKERBLEND_H
