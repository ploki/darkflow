#include "invert.h"
#include <Magick++.h>

using Magick::Quantum;

Invert::Invert(QObject *parent) :
    LutBased(parent)
{
    for ( quantum_t i = 0 ; i <= quantum_t(QuantumRange) ; ++i) {
#if 1
        m_lut[i] = QuantumRange-i;
#else
        double ev = pow(2,log2(QuantumRange+1)-log2(double(i+1)))-1;
        m_lut[i] = ev;
#endif
    }
}
