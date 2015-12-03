#ifndef WORKERDECONVOLUTION_H
#define WORKERDECONVOLUTION_H

#include <operatorworker.h>

class OpDeconvolution;

class WorkerDeconvolution : public OperatorWorker
{
public:
    WorkerDeconvolution(QThread *thread, OpDeconvolution *op);
    Photo process(const Photo &, int, int);
    void play(QVector<QVector<Photo> > inputs, int n_outputs);
};

#endif // WORKERDECONVOLUTION_H
