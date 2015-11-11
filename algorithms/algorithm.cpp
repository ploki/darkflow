#include "algorithm.h"
#include "photo.h"

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

void Algorithm::applyOn(Photo &photo)
{
    Q_UNUSED(photo);
    qWarning("Not Implemented");
}
