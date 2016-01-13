#include "whitebalance.h"
#include "photo.h"
#include <Magick++.h>

WhiteBalance::WhiteBalance(qreal temperature,
                           qreal tint,
                           bool safe,
                           QObject *parent) :
    Algorithm(true, parent),
    m_temperature(temperature),
    m_tint(tint),
    m_safe(safe),
    m_rgb()
{
    m_temperature = clamp<double>(m_temperature,2000,12000);
    Temperature_to_RGB(temperature,m_rgb);

    m_rgb[1] = m_rgb[1] / m_tint;

    m_rgb[0]=1/m_rgb[0];
    m_rgb[1]=1/m_rgb[1];
    m_rgb[2]=1/m_rgb[2];

    double minmul=m_rgb[0];
    if (minmul > m_rgb[1] ) minmul=m_rgb[1];
    if (minmul > m_rgb[2] ) minmul=m_rgb[2];
    m_rgb[0]/=minmul;
    m_rgb[1]/=minmul;
    m_rgb[2]/=minmul;

    if ( m_safe ) {
        double maxmul=1;
        if ( maxmul < m_rgb[0] ) maxmul=m_rgb[0];
        if ( maxmul < m_rgb[1] ) maxmul=m_rgb[1];
        if ( maxmul < m_rgb[2] ) maxmul=m_rgb[2];
        m_rgb[0]/=maxmul;
        m_rgb[1]/=maxmul;
        m_rgb[2]/=maxmul;
    }
}

void WhiteBalance::applyOnImage(Magick::Image& image, bool hdr)
{
    image.modifyImage();
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);
    double rgb[3];
    if (hdr) {
        rgb[0] = log2(m_rgb[0])*4096;
        rgb[1] = log2(m_rgb[1])*4096;
        rgb[2] = log2(m_rgb[2])*4096;
    }
    else {
        rgb[0] = m_rgb[0];
        rgb[1] = m_rgb[1];
        rgb[2] = m_rgb[2];
    }
#pragma omp parallel for
    for (int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( !pixels ) continue;
        for (int x = 0 ; x < w ; ++x ) {
            using Magick::Quantum;
            if (hdr) {
                pixels[x].red=clamp<double>(pixels[x].red+rgb[0],0,QuantumRange);
                pixels[x].green=clamp<double>(pixels[x].green+rgb[1],0,QuantumRange);
                pixels[x].blue=clamp<double>(pixels[x].blue+rgb[2],0,QuantumRange);
            }
            else {
                pixels[x].red=clamp<double>(pixels[x].red*rgb[0],0,QuantumRange);
                pixels[x].green=clamp<double>(pixels[x].green*rgb[1],0,QuantumRange);
                pixels[x].blue=clamp<double>(pixels[x].blue*rgb[2],0,QuantumRange);
            }
        }
        pixel_cache.sync();
    }

}

void WhiteBalance::Temperature_to_RGB(qreal T, qreal RGB[]) {
    //thanks to ufraw_routines.c from ufraw-0.11
    // and http://www.brucelindbloom.com/index.html?Eqn_T_to_xy.html

    const double XYZ_to_RGB[3][3] = {
        { 3.24071,  -0.969258,  0.0556352 },
        {-1.53726,  1.87599,    -0.203996 },
        {-0.498571, 0.0415557,  1.05707 } };

    int c;
    double xD, yD, X, Y, Z, max;
    // Fit for CIE Daylight illuminant
    if (T<= 4000) {
        xD = 0.27475e9/(T*T*T) - 0.98598e6/(T*T) + 1.17444e3/T + 0.145986;
    } else if (T<= 7000) {
        xD = -4.6070e9/(T*T*T) + 2.9678e6/(T*T) + 0.09911e3/T + 0.244063;
    } else {
        xD = -2.0064e9/(T*T*T) + 1.9018e6/(T*T) + 0.24748e3/T + 0.237040;
    }
    yD = -3*xD*xD + 2.87*xD - 0.275;

    // Fit for Blackbody using CIE standard observer function at 2 degrees
    //xD = -1.8596e9/(T*T*T) + 1.37686e6/(T*T) + 0.360496e3/T + 0.232632;
    //yD = -2.6046*xD*xD + 2.6106*xD - 0.239156;

    // Fit for Blackbody using CIE standard observer function at 10 degrees
    //xD = -1.98883e9/(T*T*T) + 1.45155e6/(T*T) + 0.364774e3/T + 0.231136;
    //yD = -2.35563*xD*xD + 2.39688*xD - 0.196035;

    X = xD/yD;
    Y = 1;
    Z = (1-xD-yD)/yD;
    max = 0;
    for (c=0; c<3; c++) {
        RGB[c] = X*XYZ_to_RGB[0][c] + Y*XYZ_to_RGB[1][c] + Z*XYZ_to_RGB[2][c];
        if (RGB[c]>max) max = RGB[c];
    }
    for (c=0; c<3; c++) RGB[c] = RGB[c]/max;
}
