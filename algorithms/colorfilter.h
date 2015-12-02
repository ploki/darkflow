#ifndef COLORFILTER_H
#define COLORFILTER_H

#include "algorithm.h"

class ColorFilter : public Algorithm
{
    Q_OBJECT
public:
    explicit ColorFilter(qreal r,
                         qreal g,
                         qreal b,
                         QObject *parent = 0);
    void applyOnImage(Magick::Image& image);

private:
    qreal m_rgb[3];
};

#endif // COLORFILTER_H
