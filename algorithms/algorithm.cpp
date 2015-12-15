#include "algorithm.h"
#include "photo.h"
#include "console.h"

#include <Magick++.h>

Algorithm::Algorithm(QObject *parent) :
    QObject(parent)
{

}

Photo Algorithm::apply(const Photo &source)
{
    Photo photo(source);
    applyOn(photo);
    return photo;
}

void Algorithm::applyOnImage(Magick::Image &)
{
    dflWarning("Algorithm::applyOnImage Not Implemented");
}

void Algorithm::applyOn(Photo &photo)
{
    Magick::Image& image = photo.image();
    applyOnImage(image);
}
