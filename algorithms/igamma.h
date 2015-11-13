#ifndef IGAMMA_H
#define IGAMMA_H

#include "lutbased.h"

class iGamma : public LutBased
{
    Q_OBJECT
public:
    explicit iGamma(qreal gamma, qreal x0, bool invert = false, QObject *parent = 0);
    static iGamma* sRGB(QObject *parent = 0);
    static iGamma* BT709(QObject *parent = 0);
    static iGamma* Lab(QObject *parent = 0);
    static iGamma* reverse_sRGB(QObject *parent = 0);
    static iGamma* reverse_BT709(QObject *parent = 0);
    static iGamma* reverse_Lab(QObject *parent = 0);
private:
    qreal m_gamma;
    qreal m_x0;
    bool m_invert;
};

#endif // IGAMMA_H
