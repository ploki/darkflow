#include "exposure.h"
#include <Magick++.h>

using Magick::Quantum;

Exposure::Exposure(qreal multiplier, QObject *parent) :
    LutBased(parent)
{
#pragma omp parallel for
    for ( unsigned int i = 0 ; i <= QuantumRange ; ++i ) {
        m_lut[i] = clamp<quantum_t>(multiplier*i, 0, QuantumRange);
    }

    qreal hdrMultiplier = log2(multiplier)*4096;
#pragma omp parallel for
    for ( unsigned int i = 0 ; i <= QuantumRange ; ++i ) {
        m_hdrLut[i] = clamp<quantum_t>(hdrMultiplier+i, 0, QuantumRange);
    }
}
