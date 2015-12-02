#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

#include "algorithm.h"

class ChannelMixer : public Algorithm
{
    Q_OBJECT
public:
    explicit ChannelMixer(qreal r,
                 qreal g,
                 qreal b,
                 QObject *parent = 0);
    void applyOnImage(Magick::Image& image);

private:
    qreal m_rgb[3];
};

#endif // CHANNELMIXER_H
