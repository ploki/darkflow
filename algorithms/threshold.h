#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "lutbased.h"

class Threshold : public LutBased
{
    Q_OBJECT
public:
    explicit Threshold(qreal high, qreal low, QObject *parent = 0);
};

#endif // THRESHOLD_H
