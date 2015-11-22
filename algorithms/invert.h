#ifndef INVERT_H
#define INVERT_H

#include "lutbased.h"

class Invert : public LutBased
{
    Q_OBJECT
public:
    Invert(QObject *parent = 0);
};

#endif // INVERT_H
