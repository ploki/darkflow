#include <cmath>
#include <Magick++.h>
#include "igamma.h"
#include "photo.h"

using Magick::Quantum;

iGamma::iGamma(qreal gamma, qreal x0, bool invert, QObject *parent) :
    QObject(parent),
    m_gamma(gamma),
    m_x0(x0),
    m_invert(invert),
    m_lut(new quantum_t[QuantumRange+1])
{
    if ( gamma == 1.0L ) return;
    double a = - ( gamma - 1.L)*pow(x0,1.L/gamma)/((gamma-1.L)*pow(x0,1.L/gamma)-gamma);
    double p=0;
    if ( invert ) {
            //recalcul de x0 et a
            x0=(1.L+a)*pow(x0,1.L/gamma)-a;
            a=(gamma-1.L)*x0;
            p=gamma*pow((x0+a)/(a+1.L),gamma)/(x0+a);
    }
    else
            p=(a+1.L)*pow(x0,1.L/gamma)/(gamma*x0);

    #pragma omp parallel for
    for ( unsigned int i = 0 ; i <= QuantumRange ; ++i ) {
            double xx= double(i)/double(QuantumRange);
            if ( xx > x0 ) {
                    if ( invert ) {
                            m_lut[i]=pow(((xx+a)/(a+1.L)),gamma)*QuantumRange;
                    }
                    else {
                            m_lut[i]=((1.L+a)*pow(xx,(1.L/gamma))-a)*QuantumRange;
                    }
            }
            else {
                    m_lut[i]=p*xx*double(QuantumRange);
            }
    }
    #pragma omp barrier

}

iGamma::~iGamma()
{
    delete[] m_lut;
}

Photo iGamma::apply(const Photo &source)
{
    Photo photo(source);
    applyOn(photo);
    return photo;
}

void iGamma::applyOn(Photo &photo)
{
    Magick::Image &image =  *photo.image();
    image.modifyImage();

    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);

#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( !pixels ) continue;
        for ( int x = 0 ; x < w ; ++x ) {
            pixels[x].red=m_lut[pixels[x].red];
            pixels[x].green=m_lut[pixels[x].green];
            pixels[x].blue=m_lut[pixels[x].blue];
        }
    }
#pragma omp barrier
    pixel_cache.sync();

}
