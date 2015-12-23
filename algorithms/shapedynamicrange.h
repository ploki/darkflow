#ifndef SHAPEDYNAMICRANGE_H
#define SHAPEDYNAMICRANGE_H

#include "lutbased.h"

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
    void applyOnImage(Magick::Image& image, bool hdr);
private:
    Shape m_shape;
    qreal m_dynamicRange;
    qreal m_exposure;
    bool m_labDomain;
};

#endif // SHAPEDYNAMICRANGE_H
