#include <cmath>
#include "shapedynamicrange.h"
#include "photo.h"
#include <Magick++.h>
#include "hdr.h"

using Magick::Quantum;
/*
    lx=10**-5
    f(x)=(sin((((x/5)*pi)+pi/2))/2.-1./2.)*(-log((lx)**(1/2.4))/log(10))
    g(x)=10**f(log(x)/log(10))
    plot [x=lx:1] x**(1./2.4),g(x**(16./(12)))

    tanh:
    set logscale xy 2
    f(x)=((tanh(2*x/(log(65536)/log(10))+1)-1-(tanh(1)-1))*log(65536)/log(10)/2)
    g(x)=10**f(log(x)/log(10))
    plot [x=1/(2.**16):1] [1/(2.**8):1] x**(1./2.4),g(x**(16./10))**(1/2.4)

*/

//sin douce
// double lx=pow(10,-5);
// #define func_f(x) ( (sin((((x/5.L)*M_PI)+M_PI/2.L))/2.L-1.L/2.L)*(-log(pow(lx,1.L/2.4L))/log(10)) )
//sin dure
// #define func_f(x) ( (sin((((x/5.L)*M_PI))))*(-log(pow     (lx,1.L/2.4L))/log(10)) )
//tanh - gamma
// #define func_f(x) ((tanh(2*x/(log(pow(2,IL_W))/log(10))+1.L)-1.L-(tanh(1)-1.L))*log(pow(2,IL_H))/log(10)/2)

static const double IL_W = 16;
static const double IL_H = 12;
static const double k1 = .30103; //???
static const double k2 = log(pow(2,IL_W))/log(10);
static const double k3 = log(pow(2,IL_H))/log(10)/2;

static inline double func_f(double x, double val) {
    return ( tanh( (val*k1) + 2.*x/k2 + 1.L )
            - 1.L
            - (tanh(1+(val*k1))-1.L)) * k3;
}
static inline double func_g(double x, double val) {
    return pow(10,func_f(log(x)/log(10),val));
}

ShapeDynamicRange::ShapeDynamicRange(ShapeDynamicRange::Shape shape, qreal dynamicRange, qreal exposure, bool labDomain, QObject *parent) :
    LutBased(parent),
    m_shape(shape),
    m_dynamicRange(dynamicRange),
    m_exposure(exposure),
    m_labDomain(labDomain)
{
    //log2(log2(DR)) may sounds weird but it was this way in not-so-original
    //FIXME verify this

    dynamicRange = log2(m_dynamicRange);
    double val = log2(m_exposure)/pow(2.,log2(dynamicRange)-3.);
    bool stop=false;
    bool hdrStop=false;
    for ( int i = QuantumRange ; i >= 0 ; --i ) {
        double xx=double(i)/QuantumRange;
        double hx=fromHDR(i)/QuantumRange;
        m_lut[i]=func_g(pow(xx,16.L/dynamicRange), val)*QuantumRange;
        m_hdrLut[i]=toHDR(func_g(pow(hx,16.L/dynamicRange), val)*QuantumRange);

        //afin d'éviter que le sin ne remonte
        if ( stop )
            m_lut[i]=m_lut[i+1];
        else if ( i!=QuantumRange && m_lut[i] > m_lut[i+1] ) {
            stop=true;
            m_lut[i]=m_lut[i+1];
        }
        if ( hdrStop )
            m_hdrLut[i]=m_hdrLut[i+1];
        else if ( i!=QuantumRange && m_hdrLut[i] > m_hdrLut[i+1] ) {
            hdrStop=true;
            m_hdrLut[i]=m_hdrLut[i+1];
        }
    }
    //fait en sorte que le pied soit toujours <=  la droite y=x afin de conserver le point noir
    //valable pour plage dynamique <= 13 car au delà, la courbe est toujours sup
}

void ShapeDynamicRange::applyOnImage(Magick::Image& image, bool hdr)
{
    int h = image.rows(),
            w = image.columns();

    image.modifyImage();
    Magick::Pixels pixel_cache(image);

#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( ! pixels ) {
            continue;
            //throw ServletException("pixel packet array is null");
        }
        for (int x = 0 ; x < w ; ++x ) {
            quantum_t rgb[3];
            rgb[0] = pixels[x].red;
            rgb[1] = pixels[x].green;
            rgb[2] = pixels[x].blue;
            if (hdr) {
             if ( m_labDomain )    {
                 double cur = .2126L*fromHDR(rgb[0]) +
                         .7152L*fromHDR(rgb[1]) +
                         .0722L*fromHDR(rgb[1]);
                 double lum = fromHDR(m_hdrLut[clamp(toHDR(cur))]);
                 double mul = log2(lum/cur)*4096;
                 rgb[0] = mul + rgb[0];
                 rgb[1] = mul + rgb[1];
                 rgb[2] = mul + rgb[2];
             }
             else {
                 rgb[0] = m_hdrLut[rgb[0]];
                 rgb[1] = m_hdrLut[rgb[1]];
                 rgb[2] = m_hdrLut[rgb[2]];
             }
            }
            else {
                if ( m_labDomain ) {
                    double cur = .2126L*rgb[0] +
                            .7152L*rgb[1] +
                            .0722L*rgb[2];
                    double lum = m_lut[clamp<quantum_t>(round(cur))];
                    double mul = lum/cur;
                    rgb[0] = mul*rgb[0];
                    rgb[1] = mul*rgb[1];
                    rgb[2] = mul*rgb[2];
                }
                else {
                    rgb[0] = m_lut[rgb[0]];
                    rgb[1] = m_lut[rgb[1]];
                    rgb[2] = m_lut[rgb[2]];
                }
            }
            pixels[x].red = clamp(rgb[0]);
            pixels[x].green = clamp(rgb[1]);
            pixels[x].blue = clamp(rgb[2]);

        }
    }
    pixel_cache.sync();
}
