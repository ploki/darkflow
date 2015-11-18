#ifndef SHAPEDYNAMICRANGE_H
#define SHAPEDYNAMICRANGE_H

#include "lutbased.h"

class iGamma;

class ShapeDynamicRange : public LutBased
{
    Q_OBJECT
public:
    typedef enum {
        TanH
    } Shape;
    ShapeDynamicRange(Shape shape,
                      qreal dynamicRange,
                      qreal exposure,
                      bool labDomain,
                      QObject *parent = 0);
    void applyOnImage(Magick::Image& image);
private:
    Shape m_shape;
    qreal m_dynamicRange;
    qreal m_exposure;
    bool m_labDomain;
    iGamma& m_LabGamma;
    iGamma& m_labGammaReverse;
};

#endif // SHAPEDYNAMICRANGE_H
