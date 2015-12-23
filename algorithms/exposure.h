#ifndef EXPOSURE_H
#define EXPOSURE_H

#include "lutbased.h"

class Exposure : public LutBased
{
    Q_OBJECT
public:
    explicit Exposure(qreal multiplier, QObject *parent = 0);
};

#endif // EXPOSURE_H
