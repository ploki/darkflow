#include "algorithm.h"
#include "photo.h"
#include "console.h"

#include <Magick++.h>

Algorithm::Algorithm(bool alterCurve, QObject *parent) :
    QObject(parent),
    m_alterCurve(alterCurve)
{

}

void Algorithm::applyOnImage(Magick::Image &, bool)
{
    dflWarning("Algorithm::applyOnImage Not Implemented");
}

void Algorithm::applyOn(Photo &photo)
{
    bool hdr = photo.getScale() == Photo::HDR;
    applyOnImage(photo.image(), hdr);
    if (m_alterCurve)
        applyOnImage(photo.curve(), hdr);
}
