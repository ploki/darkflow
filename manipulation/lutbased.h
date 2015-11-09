#ifndef LUTBASED_H
#define LUTBASED_H

#include <QObject>
#include "manipulation.h"
#include "photo.h"


class LutBased : public Manipulation
{
    Q_OBJECT
public:
    explicit LutBased(QObject *parent = 0);
    ~LutBased();

    void applyOn(Photo& photo);

protected:
    quantum_t *m_lut;
};

#endif // LUTBASED_H
