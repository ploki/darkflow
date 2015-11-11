#include "exposure.h"
#include <Magick++.h>

using Magick::Quantum;

Exposure::Exposure(qreal multiplier, QObject *parent) :
    LutBased(parent),
    m_multiplier(multiplier)
{
#pragma omp parallel for
    for ( unsigned int i = 0 ; i <= QuantumRange ; ++i ) {
        m_lut[i] = clamp<quantum_t>(m_multiplier*i, 0, QuantumRange);
    }
#pragma omp barrier
}
