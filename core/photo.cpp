#include "photo.h"
#include <QFile>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <Magick++.h>

#include <string>

#include "igamma.h"
#include "exposure.h"

using namespace Magick;

Photo::Photo(QObject *parent) :
    QObject(parent),
    m_image(NULL),
    m_error(true),
    m_tags(),
    m_sequenceNumber(0)
{
}

Photo::Photo(const Blob &blob, QObject *parent) :
    QObject(parent),
    m_image(NULL),
    m_error(false),
    m_tags(),
    m_sequenceNumber(0)
{
    try {
        m_image = new Magick::Image(blob);
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
    }
}

Photo::Photo(const Magick::Image *image, QObject *parent) :
    QObject(parent),
    m_image(NULL),
    m_error(false),
    m_tags(),
    m_sequenceNumber(0)
{
    try {
        m_image = new Magick::Image(*image);
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
    }
}

Photo::Photo(const Photo &photo) :
    QObject(photo.parent()),
    m_image(NULL),
    m_error(false),
    m_tags(photo.m_tags),
    m_sequenceNumber(photo.m_sequenceNumber)
{
    try {
        m_image = new Magick::Image(*photo.m_image);
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
    }
}

Photo::~Photo()
{
    delete m_image;
}

Photo &Photo::operator=(const Photo &photo)
{
    Magick::Image *newImage;
    try {
        newImage = new Magick::Image(*photo.m_image);
        m_image = newImage;
        m_tags = photo.m_tags;
        m_sequenceNumber = photo.m_sequenceNumber;
        m_error = false;
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
    }
    return *this;
}

bool Photo::load(const QString &filename)
{
    QFile file(filename);
    if ( !file.open(QFile::ReadOnly) )
        return false;
    QByteArray data = file.readAll();
    Blob blob(data.data(), data.length());
    try {
        delete m_image;
        m_image = new Magick::Image(blob);
        m_error = false;
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
        return false;
    }
    return true;
}

bool Photo::save(const QString &filename, const QString &magick)
{
    Blob blob;
    try {
        m_image->write(&blob, magick.toStdString());
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
        return false;
    }

    QFile file(filename);
    file.open(QFile::WriteOnly);
    if ( !file.write(reinterpret_cast<const char*>(blob.data()),blob.length()) ) {
        setError();
        return false;
    }
    return true;
}

void Photo::create(long width, long height)
{
    try {
        delete m_image;
        m_image = new Magick::Image(Geometry(width,height),Color(0,0,0));
        m_error = false;
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
    }
}

void Photo::createAlike(const Photo *photo)
{
    create(photo->m_image->columns(), photo->m_image->rows());
}

bool Photo::error() const
{
    return m_error;
}

Magick::Image *Photo::image() const
{
    return m_image;
}

QMap<QString, QString> Photo::tags() const
{
    return m_tags;
}

void Photo::setTag(const QString &name, const QString &value)
{
    m_tags.insert(name, value);
}

void Photo::removeTag(const QString &name)
{
    m_tags.remove(name);
}

QString Photo::getTag(const QString &name) const
{
    QMap<QString, QString>::const_iterator it = m_tags.find(name);
    if ( it == m_tags.end() )
        return QString();
    return it.value();
}

void Photo::setError()
{
    m_error = true;
    delete m_image;
    m_image = NULL;
}

QPixmap Photo::toPixmap(double gamma, double x0, double exposureBoost)
{
    Q_UNUSED(exposureBoost);
    Photo photo(*this);
    Q_UNUSED(gamma);
    Q_UNUSED(x0);
    Exposure(exposureBoost).applyOn(photo);
    iGamma(gamma, x0).applyOn(photo);

    Magick::Image &image = *photo.image();
    image.modifyImage();
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);
    QImage qImage(w, h, QImage::Format_RGB32);
    //#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        const Magick::PixelPacket *pixels = pixel_cache.getConst(0,y,w,1);
        if ( !pixels ) continue;
        for ( int x = 0 ; x < w ; ++x ) {
            qImage.setPixel(x,y,qRgb(
                                (pixels[x].red/256),
                                (pixels[x].green/256),
                                (pixels[x].blue/256)));
        }
    }
    // #pragma omb barrier
    return QPixmap::fromImage(qImage,Qt::AutoColor|Qt::AvoidDither);
}

void Photo::writeJPG(const QString &filename)
{
    Magick::Image image(*this->image());

    image.magick("JPG");
    Magick::Blob blob;
    image.write(&blob);
    QFile f(filename);
    f.open(QFile::WriteOnly);
    f.write((char*)blob.data(), blob.length());
}

int Photo::getSequenceNumber() const
{
    return m_sequenceNumber;
}

void Photo::setSequenceNumber(int sequenceNumber)
{
    m_sequenceNumber = sequenceNumber;
}

bool Photo::operator<(const Photo &other) const {
    return (m_sequenceNumber < other.m_sequenceNumber);
}



