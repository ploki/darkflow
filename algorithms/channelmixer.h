#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

#include "algorithm.h"

class ChannelMixer : public Algorithm
{
    Q_OBJECT
public:
    explicit ChannelMixer(qreal r = .2126L,
                 qreal g = .7152L,
                 qreal b = .0722L,
                 QObject *parent = 0);
    void applyOnImage(Magick::Image& image, bool hdr);

private:
    qreal m_rgb[3];
};

#endif // CHANNELMIXER_H
