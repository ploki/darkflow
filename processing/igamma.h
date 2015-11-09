#ifndef IGAMMA_H
#define IGAMMA_H

#include <QObject>

#include "photo.h"

class iGamma : public QObject
{
    Q_OBJECT
public:
    explicit iGamma(qreal gamma, qreal x0, bool invert = false, QObject *parent = 0);
    ~iGamma();

    Photo apply(const Photo& source);
    void applyOn(Photo& photo);

signals:

public slots:
private:
    qreal m_gamma;
    qreal m_x0;
    bool m_invert;
    quantum_t *m_lut;
};

#endif // IGAMMA_H
