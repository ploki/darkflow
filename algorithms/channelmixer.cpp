#include "channelmixer.h"
#include "photo.h"
#include <Magick++.h>
#include "hdr.h"

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
    image.modifyImage();
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);
#pragma omp parallel for
    for (int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( !pixels ) continue;
        for (int x = 0 ; x < w ; ++x ) {
            using Magick::Quantum;
            if (hdr) {
                pixels[x].red=
                pixels[x].green=
                pixels[x].blue=clamp<quantum_t>(
                            toHDR(m_rgb[0]*fromHDR(pixels[x].red) +
                                  m_rgb[1]*fromHDR(pixels[x].green) +
                                  m_rgb[2]+fromHDR(pixels[x].blue)));
            }
            else {
                pixels[x].red=
                pixels[x].green=
                pixels[x].blue=clamp<quantum_t>(
                            round(m_rgb[0]*pixels[x].red +
                                  m_rgb[1]*pixels[x].green +
                                  m_rgb[2]*pixels[x].blue));
            }
        }
    }
    pixel_cache.sync();

}
