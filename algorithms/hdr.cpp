#include "hdr.h"
#include <Magick++.h>

using Magick::Quantum;



HDR::HDR(bool revert, QObject *parent) :
    LutBased(parent)
{

#pragma omp parallel for
    for(unsigned i = 0 ; i <= QuantumRange ; ++i ) {
        m_hdrLut[i] = clamp( revert
                          ? round(fromHDR(i))
                          : i);
    }
#pragma omp parallel for
    for(unsigned i = 0 ; i <= QuantumRange ; ++i ) {
        m_lut[i] = clamp( revert
                          ? i
                          : toHDR(i));
    }
}
