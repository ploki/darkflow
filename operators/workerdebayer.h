#ifndef WORKERDEBAYER_H
#define WORKERDEBAYER_H

#include "operatorworker.h"
#include "opdebayer.h"

class WorkerDebayer : public OperatorWorker
{
    Q_OBJECT
public:
    WorkerDebayer(OpDebayer::Debayer quality, QThread *thread, Operator *op);
    Photo process(const Photo &photo, int p, int c);
    bool play_onInput(int idx);

private:
    OpDebayer::Debayer m_quality;
};

#endif // WORKERDEBAYER_H
