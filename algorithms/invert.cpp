#include "invert.h"
#include <Magick++.h>
#include "hdr.h"
using Magick::Quantum;

Invert::Invert(QObject *parent) :
    LutBased(parent)
{
#pragma omp parallel for
    for ( quantum_t i = 0 ; i <= quantum_t(QuantumRange) ; ++i) {
        m_lut[i] = QuantumRange-i;
        m_hdrLut[i] = toHDR(double(QuantumRange)-fromHDR(i));
    }
}
