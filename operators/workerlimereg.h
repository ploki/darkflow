#ifndef WORKERLIMEREG_H
#define WORKERLIMEREG_H

#include "operatorworker.h"
struct Limereg_TrafoParams;
struct Limereg_Image;

class WorkerLimereg : public OperatorWorker
{
public:
    WorkerLimereg(qreal maxRotation, qreal maxTranslation, QThread *thread, Operator *op);

    Photo process(const Photo &photo, int, int);
    void play_analyseSources();
    bool play_onInput(int idx);

private:
    qreal m_maxRotation;
    qreal m_maxTranslation;
    int m_refIdx;

    bool registerLimereg(Limereg_Image& reference, Limereg_Image& limg, Limereg_TrafoParams& result);
};

#endif // WORKERLIMEREG_H
