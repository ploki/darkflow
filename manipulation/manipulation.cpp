#include "manipulation.h"
#include "photo.h"

Manipulation::Manipulation(QObject *parent) :
    QObject(parent)
{

}

Photo Manipulation::apply(const Photo &source)
{
    Photo photo(source);
    applyOn(photo);
    return photo;
}

void Manipulation::applyOn(Photo &photo)
{
    Q_UNUSED(photo);
    qWarning("Not Implemented");
}
