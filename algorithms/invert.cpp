#include "invert.h"
#include <Magick++.h>
#include "hdr.h"
using Magick::Quantum;

Invert::Invert(QObject *parent) :
    LutBased(parent)
{
#pragma omp parallel for
    for ( int i = 0 ; i <= int(QuantumRange) ; ++i) {
        m_lut[i] = QuantumRange-i;
        m_hdrLut[i] = toHDR(double(QuantumRange)-fromHDR(i));
    }
}
