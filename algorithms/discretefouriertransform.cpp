#include "discretefouriertransform.h"
#include <QObject>
#include <Magick++.h>
#include "photo.h"
#include "algorithm.h"
#include "console.h"

using Magick::Quantum;

Q_STATIC_ASSERT( sizeof(fftw_complex) == sizeof(std::complex<double>));

__attribute__((constructor))
static void
init()
{
    fftw_init_threads();
}

DiscreteFourierTransform::DiscreteFourierTransform(Magick::Image &image)
    : m_w(image.columns()),
      m_h(image.rows()),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    std::complex<double> *input = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    //std::complex<double> *output = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    Ordinary::Pixels cache(image);
    const Magick::PixelPacket *pixels = cache.getConst(0, 0, m_w, m_h);
    for (int c = 0 ; c < 3 ; ++c ) {
        for ( int y = 0 ; y < m_h ; ++y ) {
            for ( int x = 0 ; x < m_w ; ++x ) {
                double pixel = 0;
                switch(c) {
                case 0: pixel = double(pixels[y*m_w+x].red)/QuantumRange; break;
                case 1: pixel = double(pixels[y*m_w+x].green)/QuantumRange; break;
                case 2: pixel = double(pixels[y*m_w+x].blue)/QuantumRange; break;
                }
                input[y*m_w+x] = std::complex<double>(pixel, 0);
            }
        }
        std::complex<double> *plane = 0;
        switch(c) {
        case 0: plane = red; break;
        case 1: plane = green; break;
        case 2: plane = blue; break;
        }
        fftw_plan plan = fftw_plan_dft_2d(m_h, m_w,
                                          reinterpret_cast<fftw_complex*>(input),
                                          reinterpret_cast<fftw_complex*>(plane), FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
//        memcpy(plane, output, sizeof(fftw_complex)*m_h*m_w);
        fftw_destroy_plan(plan);
    }
    //fftw_free(output);
    fftw_free(input);
}

DiscreteFourierTransform::~DiscreteFourierTransform()
{
    fftw_free(red);
    fftw_free(green);
    fftw_free(blue);
}

Magick::Image DiscreteFourierTransform::reverse(double luminosity)
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    std::complex<double> *input = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    std::complex<double> *output = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    for ( int c = 0 ; c < 3 ; ++c ) {
        std::complex<double> *plane = 0;
        switch(c) {
        case 0: plane = red; break;
        case 1: plane = green; break;
        case 2: plane = blue; break;
        }
        memcpy(input, plane, sizeof(fftw_complex)*m_h*m_w);
        fftw_plan plan = fftw_plan_dft_2d(m_h, m_w,
                                          reinterpret_cast<fftw_complex*>(input),
                                          reinterpret_cast<fftw_complex*>(output), FFTW_BACKWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
        for ( int y = 0 ; y < m_h ; ++y ) {
            for ( int x = 0 ; x < m_w ; ++x ) {
                quantum_t pixel = clamp(luminosity*output[y*m_w+x].real()*QuantumRange/(m_w*m_h));
                switch(c) {
                case 0: pixels[y*m_w+x].red = pixel; break;
                case 1: pixels[y*m_w+x].green = pixel; break;
                case 2: pixels[y*m_w+x].blue = pixel; break;
                }
            }
        }
        fftw_destroy_plan(plan);
    }
    cache.sync();
    fftw_free(output);
    fftw_free(input);
    return image;
}

Magick::Image DiscreteFourierTransform::imageMagnitude()
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            int xx = (x+m_w/2)%m_w;
            int yy = (y+m_h/2)%m_h;
            pixels[yy*m_w+xx].red = clamp<quantum_t>( std::abs(red[y*m_w+x]) * QuantumRange );
            pixels[yy*m_w+xx].green = clamp<quantum_t>( std::abs(green[y*m_w+x]) * QuantumRange );
            pixels[yy*m_w+xx].blue = clamp<quantum_t>( std::abs(blue[y*m_w+x]) * QuantumRange );
        }
    }
    cache.sync();
    return image;
}

Magick::Image DiscreteFourierTransform::imagePhase()
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            int xx = (x+m_w/2)%m_w;
            int yy = (y+m_h/2)%m_h;
            pixels[yy*m_w+xx].red = clamp<quantum_t>( (std::arg(red[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
            pixels[yy*m_w+xx].green = clamp<quantum_t>( (std::arg(green[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
            pixels[yy*m_w+xx].blue = clamp<quantum_t>( (std::arg(blue[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
        }
    }
    cache.sync();
    return image;
}

DiscreteFourierTransform &DiscreteFourierTransform::operator/=(const DiscreteFourierTransform &other)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] /=  ( other.red[i] == 0. ? 1e-12 : other.red[i]);
        green[i] /= ( other.green[i] == 0. ? 1e-12 : other.green[i]);
        blue[i] /= ( other.blue[i] == 0. ? 1e-12 : other.blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::operator*=(const DiscreteFourierTransform &other)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] *= other.red[i];
        green[i] *= other.green[i];
        blue[i] *= other.blue[i];
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::conj()
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::conj(red[i]);
        green[i] = std::conj(green[i]);
        blue[i] = std::conj(blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::inv()
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = 1. / ( red[i] == 0. ? 1e-12 : red[i]);
        green[i] = 1. / ( green[i] == 0. ? 1e-12 : green[i]);
        blue[i] = 1. / ( blue[i] == 0. ? 1e-12 : blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::wienerFilter(double k)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::conj(red[i])/(pow(std::abs(red[i]),2)+k);
        green[i] = std::conj(green[i])/(pow(std::abs(green[i]),2)+k);
        blue[i] = std::conj(blue[i])/(pow(std::abs(blue[i]),2)+k);
    }
    return *this;
}

DiscreteFourierTransform::DiscreteFourierTransform(const DiscreteFourierTransform &other)
    : m_w(other.m_w),
      m_h(other.m_h),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    memcpy(red, other.red, sizeof(fftw_complex)*m_h*m_w);
    memcpy(green, other.green, sizeof(fftw_complex)*m_h*m_w);
    memcpy(blue, other.blue, sizeof(fftw_complex)*m_h*m_w);
}
