#ifndef LUTBASED_H
#define LUTBASED_H

#include <QObject>
#include "algorithm.h"
#include "photo.h"


class LutBased : public Algorithm
{
    Q_OBJECT
public:
    explicit LutBased(QObject *parent = 0);
    ~LutBased();

    void applyOnImage(Magick::Image& image);
    quantum_t applyOnQuantum(quantum_t v);

protected:
    quantum_t *m_lut;
};

#endif // LUTBASED_H
