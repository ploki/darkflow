#ifndef HOTPIXELS_H
#define HOTPIXELS_H

#include <QObject>
#include "algorithm.h"

class HotPixels : public Algorithm
{
    Q_OBJECT
public:
    HotPixels(double delta, bool aggressive, bool  naive, QObject *parent = 0);
    void applyOnImage(Magick::Image &image);
private:
    double m_delta;
    bool m_aggressive;
    bool m_naive;
};

#endif // HOTPIXELS_H
