#ifndef WORKERCONVOLUTION_H
#define WORKERCONVOLUTION_H

#include <operatorworker.h>

class OpConvolution;

class WorkerConvolution : public OperatorWorker
{
public:
    WorkerConvolution(qreal luminosity, QThread *thread, OpConvolution *op);
    Photo process(const Photo &, int, int);
    void play();
private:
    qreal m_luminosity;
};

#endif // WORKERCONVOLUTION_H