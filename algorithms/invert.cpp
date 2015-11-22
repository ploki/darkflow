#include "invert.h"
#include <Magick++.h>

using Magick::Quantum;

Invert::Invert(QObject *parent) :
    LutBased(parent)
{
    for ( quantum_t i = 0 ; i <= quantum_t(QuantumRange) ; ++i) {
        m_lut[i] = double(QuantumRange)/double(i);
    }
}
