#include "operatorexnihilo.h"
#include "process.h"
#include "image.h"


OperatorExNihilo::OperatorExNihilo(Process *parent) :
    Operator(parent)
{
}

OperatorExNihilo::~OperatorExNihilo()
{
    //qWarning((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}

OperatorExNihilo *OperatorExNihilo::newInstance()
{
    return new OperatorExNihilo(m_process);
}

QString OperatorExNihilo::getClassIdentifier()
{
    return "Ex Nihilo";
}

void OperatorExNihilo::play()
{
    setUpToDate(false);
    Image *image = process(NULL);
    m_result.push_back(image);
}

Image *OperatorExNihilo::process(const Image *image)
{
    Q_UNUSED(image);
    QString filename = m_process->temporaryDirectory() + "/";
    filename += Image::NewRandomName();
    Image *newImage = new Image(filename);
    newImage->create(1000,1000,Image::sRGB, Image::Pixel_u16);
    Image::Triplet<Image::u16> *pixels =
            newImage->getPixelsTriplet<Image::u16>();
    for (int y = 0 ; y < newImage->height() ; ++y)
        for (int x = 0 ; x < newImage->width() ; ++x ) {
            pixels[y*newImage->width()+x] = Image::Triplet<Image::u16>(12,128,250);
        }
    newImage->save();
    return newImage;
}

