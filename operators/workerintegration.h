#ifndef WORKERINTEGRATION_H
#define WORKERINTEGRATION_H

#include "operatorworker.h"
#include "opintegration.h"

namespace Magick {
class Image;
}

class WorkerIntegration : public OperatorWorker
{
public:
    WorkerIntegration(OpIntegration::RejectionType rejectionType,
                      qreal upper,
                      qreal lower,
                      OpIntegration::NormalizationType normalizationType,
                      qreal customNormalizationValue,
                      QThread *thread, OpIntegration *op);
    ~WorkerIntegration();
    Photo process(const Photo &, int, int) { throw 0; }
    bool play_onInput(int idx);

    QRectF computePlanesDimensions();
signals:

public slots:
    OpIntegration::RejectionType m_rejectionType;
    qreal m_upper;
    qreal m_lower;
    OpIntegration::NormalizationType m_normalizationType;
    qreal m_customNormalizationValue;
    quantum_t *m_integrationPlane;
    int *m_countPlane;
    int m_w;
    int m_h;
    qreal m_offX;
    qreal m_offY;

private:
    void createPlanes(Magick::Image&);
};

#endif // WORKERINTEGRATION_H
