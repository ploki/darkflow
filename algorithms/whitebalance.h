#ifndef WHITEBALANCE_H
#define WHITEBALANCE_H

#include "algorithm.h"

class WhiteBalance : public Algorithm
{
    Q_OBJECT
public:
    explicit WhiteBalance(qreal temperature,
                          qreal tint,
                          bool safe,
                          QObject *parent = 0);

    void applyOnImage(Magick::Image& image);
signals:

public slots:

private:
    qreal m_temperature;
    qreal m_tint;
    bool m_safe;
    qreal m_rgb[3];

    static void Temperature_to_RGB(qreal T, qreal RGB[3]);
};

#endif // WHITEBALANCE_H
