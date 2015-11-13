#include <cmath>
#include <Magick++.h>
#include "igamma.h"
#include "photo.h"

using Magick::Quantum;

iGamma::iGamma(qreal gamma, qreal x0, bool invert, QObject *parent) :
    LutBased(parent),
    m_gamma(gamma),
    m_x0(x0),
    m_invert(invert)
{
    //FIXME doesn't work in this model
    //if ( gamma == 1.0L ) return;
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
                m_lut[i]= ((1.L+a)*pow(xx,(1.L/gamma))-a)*QuantumRange;
            }
        }
        else {
            m_lut[i]=p*xx*double(QuantumRange);
        }
    }
#pragma omp barrier

}

iGamma* iGamma::sRGB(QObject *parent)
{
    return new iGamma(2.4L, 0.00304L,false, parent);
}

iGamma* iGamma::BT709(QObject *parent)
{
    return new iGamma(2.222L, 0.018L,false, parent);
}

iGamma* iGamma::Lab(QObject *parent)
{
    return new iGamma(3.0L, 0.008856L, false, parent);
}

iGamma* iGamma::reverse_sRGB(QObject *parent)
{
    return new iGamma(2.4L, 0.00304L,true, parent);
}

iGamma* iGamma::reverse_BT709(QObject *parent)
{
    return new iGamma(2.222L, 0.018L,true, parent);
}

iGamma* iGamma::reverse_Lab(QObject *parent)
{
    return new iGamma(3.0L, 0.008856L, true, parent);
}

