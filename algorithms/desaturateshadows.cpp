#include <cmath>
#include "desaturateshadows.h"
#include "photo.h"
#include <Magick++.h>
#include "cielab.h"

using Magick::Quantum;

DesaturateShadows::DesaturateShadows(qreal highlightLimit,
                                     qreal range,
                                     qreal saturation,
                                     QObject *parent) :
    Algorithm(parent),
    m_lut(new double[QuantumRange+1]),
    m_highlightLimit(highlightLimit),
    m_range(range),
    m_saturation(saturation)
{
    double high = 16.L + log2(highlightLimit);
    double low = high - log2(range);
    double threshold_high = highlightLimit * QuantumRange;
    double threshold_low =  threshold_high / range;
    for ( unsigned int i = 0 ; i < QuantumRange+1 ; ++i ) {
        if ( i < threshold_low ) {
            m_lut[i]=saturation;
        }
        else if ( i > threshold_high ) {
            m_lut[i]=1.;
        }
        else {
            //lut[i] = (sin(M_PI/(low-high)*(high+log(i/double(QuantumRange))/log(2))+M_PI/2.)+1.)/2.;
            m_lut[i] = ( saturation - 1. ) * (1.-(sin(M_PI/(low-high)*(high+log(i/double(QuantumRange))/log(2))+M_PI/2.)+1.)/2.) + 1.;
        }
    }
}

DesaturateShadows::~DesaturateShadows()
{
    delete[] m_lut;
}

void DesaturateShadows::applyOnImage(Magick::Image& image)
{
    int h = image.rows(),
            w = image.columns();

    image.modifyImage();
    Magick::Pixels pixel_cache(image);
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( !pixels ) continue;
        for ( int x = 0 ; x < w ; ++x ) {
            quantum_t rgb[3];
            rgb[0]=pixels[x].red;
            rgb[1]=pixels[x].green;
            rgb[2]=pixels[x].blue;
            double lab[3];
            RGB_to_LinearLab(rgb,lab);
            quantum_t L=lab[0]*QuantumRange;
            if ( L > QuantumRange ) L=QuantumRange;
            if ( ! equals(m_lut[L],1.) )
            {
                lab[1]*=m_lut[L];
                lab[2]*=m_lut[L];
                LinearLab_to_RGB(lab,rgb);
                pixels[x].red=rgb[0];
                pixels[x].green=rgb[1];
                pixels[x].blue=rgb[2];
            }
        }
    }
#pragma omp barrier
    pixel_cache.sync();
}

bool DesaturateShadows::equals(double x, double y, double prec) {

    return fabs(x-y) < prec;
}