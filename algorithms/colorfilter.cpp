#include "colorfilter.h"

#include "photo.h"
#include <Magick++.h>

ColorFilter::ColorFilter(qreal r, qreal g, qreal b, QObject *parent) :
    Algorithm(true, parent),
    m_rgb()
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}

void ColorFilter::applyOnImage(Magick::Image &image, bool hdr)
{
    image.modifyImage();
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);
    qreal rgb[3];
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
