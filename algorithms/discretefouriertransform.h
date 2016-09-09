#ifndef DISCRETEFOURIERTRANSFORM_H
#define DISCRETEFOURIERTRANSFORM_H

#include <complex>
#include <fftw3.h>

namespace Magick {
class Image;
}
class DiscreteFourierTransform
{
    int m_w;
    int m_h;
    std::complex<double> *red;
    std::complex<double> *green;
    std::complex<double> *blue;
public:
    DiscreteFourierTransform(Magick::Image& image);
    ~DiscreteFourierTransform();
    Magick::Image reverse(double luminosity);
    Magick::Image imageMagnitude();
    Magick::Image imagePhase();

    DiscreteFourierTransform& operator/=(const DiscreteFourierTransform& other);
    DiscreteFourierTransform& operator*=(const DiscreteFourierTransform& other);

    DiscreteFourierTransform& conj();
    DiscreteFourierTransform& inv();
    DiscreteFourierTransform& wienerFilter(double k);

    DiscreteFourierTransform(const DiscreteFourierTransform &other);
};

#endif // DISCRETEFOURIERTRANSFORM_H
