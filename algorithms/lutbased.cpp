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
    Magick::Image srcImage(image);
    ResetImage(image);
    quantum_t *lut = hdr ? m_hdrLut : m_lut;
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels src_cache(srcImage);
    Magick::Pixels pixel_cache(image);
    bool error=false;
#pragma omp parallel for dfl_threads(4, srcImage, image)
    for ( int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        const Magick::PixelPacket *src = src_cache.getConst(0,y,w,1);
        if ( error || !pixels || !src ) {
            if ( !error )
                dflError(DF_NULL_PIXELS);
            error=true;
            continue;
        }
        for ( int x = 0 ; x < w ; ++x ) {
            pixels[x].red=lut[src[x].red];
            pixels[x].green=lut[src[x].green];
            pixels[x].blue=lut[src[x].blue];
        }
        pixel_cache.sync();
    }
}

quantum_t LutBased::applyOnQuantum(quantum_t v, bool hdr)
{
    return (hdr ? m_hdrLut : m_lut)[clamp(v)];
}

