#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

#include "algorithm.h"
#include "cielab.h"

class ChannelMixer : public Algorithm
{
    Q_OBJECT
public:
    explicit ChannelMixer(qreal r = LUMINANCE_RED,
                 qreal g = LUMINANCE_GREEN,
                 qreal b = LUMINANCE_BLUE,
                 QObject *parent = 0);
    void applyOnImage(Magick::Image& image, bool hdr);

private:
    qreal m_rgb[3];
};

#endif // CHANNELMIXER_H
