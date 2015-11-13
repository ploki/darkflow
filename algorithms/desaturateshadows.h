#ifndef DESATURATESHADOWS_H
#define DESATURATESHADOWS_H

#include "algorithm.h"

class DesaturateShadows : public Algorithm
{
    Q_OBJECT
public:
    /**
     * @brief DesaturateShadows
     * @param highlightLimit (-16EV..0EV)
     * @param range          (0EV..16EV) number of EV on which saturation extinction takes place
     * @param saturation     (0%..100%)
     * @param parent
     */
    explicit DesaturateShadows(qreal highlightLimit,
                               qreal range,
                               qreal saturation,
                               QObject *parent = 0);
    ~DesaturateShadows();
    void applyOnImage(Magick::Image& image);
private:
    double *m_lut;
    qreal m_highlightLimit;
    qreal m_range;
    qreal m_saturation;
    bool equals(double x, double y, double prec=0.00001);
};

#endif // DESATURATESHADOWS_H
