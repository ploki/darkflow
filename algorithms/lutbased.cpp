#include "lutbased.h"
#include "console.h"
#include <Magick++.h>

using Magick::Quantum;

LutBased::LutBased(QObject *parent) :
    Algorithm(true, parent),
    m_lut(new quantum_t[QuantumRange+1]),
    m_hdrLut(new quantum_t[QuantumRange+1])
{

}

LutBased::~LutBased()
{
    delete[] m_hdrLut;
    delete[] m_lut;
}

void LutBased::applyOnImage(Magick::Image &image, bool hdr)
{
    image.modifyImage();
    quantum_t *lut = hdr ? m_hdrLut : m_lut;
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( !pixels ) {
            dflError("NULL pixels !");
            continue;
        }
        for ( int x = 0 ; x < w ; ++x ) {
            pixels[x].red=lut[pixels[x].red];
            pixels[x].green=lut[pixels[x].green];
            pixels[x].blue=lut[pixels[x].blue];
        }
    }
    pixel_cache.sync();
}

quantum_t LutBased::applyOnQuantum(quantum_t v, bool hdr)
{
    return (hdr ? m_hdrLut : m_lut)[clamp(v)];
}

