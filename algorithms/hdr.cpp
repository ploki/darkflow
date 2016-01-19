#include "hdr.h"
#include <Magick++.h>

using Magick::Quantum;

double *fromHDRLut;

class onStart {
public:
    onStart() {
        fromHDRLut = new double[65536];
        for ( int v = 0 ; v <= 65535 ; ++v ) {
#if 1
            if ( v == 0 ) fromHDRLut[v]=0;
            else fromHDRLut[v] = pow(2,(double)(v+1)/4096)-1;
#else
            if ( v == 0 ) return 0;
            return pow(2,(double)(v)/4096);
#endif
        }
    }
    ~onStart() {
        delete[] fromHDRLut;
    }
} once;


HDR::HDR(bool revert, QObject *parent) :
    LutBased(parent),
    m_revert(revert)
{

#pragma omp parallel for dfl_threads(1024)
    for(int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        m_hdrLut[i] = clamp( revert
                          ? DF_ROUND(fromHDR(i))
                          : i);
    }
#pragma omp parallel for dfl_threads(1024)
    for(int i = 0 ; i <= int(QuantumRange) ; ++i ) {
        m_lut[i] = clamp( revert
                          ? i
                          : toHDR(i));
    }
}

void HDR::applyOn(Photo &photo)
{
    bool hdr = photo.getScale() == Photo::HDR;
    applyOnImage(photo.image(), hdr);
    if (m_alterCurve)
        applyOnImage(photo.curve(), hdr);
    photo.setTag(TAG_SCALE,
                 m_revert
                 ? TAG_SCALE_LINEAR
                 : TAG_SCALE_HDR );
}
