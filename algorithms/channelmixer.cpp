#include "channelmixer.h"
#include "photo.h"
#include <Magick++.h>
#include "hdr.h"
#include "console.h"

ChannelMixer::ChannelMixer(qreal r, qreal g, qreal b, QObject *parent) :
    Algorithm(true, parent),
    m_rgb()
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}

void ChannelMixer::applyOnImage(Magick::Image &image, bool hdr)
{
    Magick::Image srcImage(image);
    ResetImage(image);
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels src_cache(srcImage);
    Magick::Pixels pixel_cache(image);
    bool error=false;
#pragma omp parallel for dfl_threads(4, srcImage, image)
    for (int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        const Magick::PixelPacket *src = src_cache.getConst(0,y,w,1);
        if ( error || !pixels || !src ) {
            if ( !error )
                dflError(DF_NULL_PIXELS);
            error=true;
            continue;
        }
        for (int x = 0 ; x < w ; ++x ) {
            using Magick::Quantum;
            if (hdr) {
                pixels[x].red=
                pixels[x].green=
                pixels[x].blue=clamp<quantum_t>(
                            toHDR(m_rgb[0]*fromHDR(src[x].red) +
                                  m_rgb[1]*fromHDR(src[x].green) +
                                  m_rgb[2]+fromHDR(src[x].blue)));
            }
            else {
                pixels[x].red=
                pixels[x].green=
                pixels[x].blue=clamp<quantum_t>(
                            round(m_rgb[0]*src[x].red +
                                  m_rgb[1]*src[x].green +
                                  m_rgb[2]*src[x].blue));
            }
        }
        pixel_cache.sync();
    }
}
