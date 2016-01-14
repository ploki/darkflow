#include "exposure.h"
#include <Magick++.h>

using Magick::Quantum;

Exposure::Exposure(qreal multiplier, QObject *parent) :
    LutBased(parent)
{
#pragma omp parallel for dfl_threads(1024)
    for ( int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        m_lut[i] = clamp<quantum_t>(multiplier*i, 0, QuantumRange);
    }

    qreal hdrMultiplier = log2(multiplier)*4096;
#pragma omp parallel for dfl_threads(1024)
    for ( int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        m_hdrLut[i] = clamp<quantum_t>(hdrMultiplier+i, 0, QuantumRange);
    }
}
