#include "image.h"
#include <QFile>
#include <QUuid>
#include <QString>

struct __attribute__((packed)) ImageImpl
{
    long magic;
    long rows;
    long columns;
    Image::ColorSpace colorSpace;
    Image::PixelSize pixelSize;
    uchar pixels[];
} ;

QString Image::NewRandomName()
{
    QUuid uuid = uuid.createUuid();
    return uuid.toString();
}
int Image::ColorSpaceComponents(ColorSpace colorspace)
{
    switch(colorspace) {
    case LinearMono:
    case sMono:
        return 1;
    default:
        return 3;
    }
}

Image::Image(const QString& filename, bool owner) :
    m_impl(0),
    m_file(new QFile(filename)),
    m_filename(filename),
    m_owner(owner)
{

}

void Image::close()
    {
    if ( m_notbacked)
        delete[] reinterpret_cast<char*>(m_impl);

    m_impl = 0;
    m_notbacked=true;
    if ( m_file )
        delete m_file;

    m_file = new QFile(m_filename);
}

bool Image::open()
{
    bool ret;
    close();
    ret = m_file->open(QFile::ReadWrite);
    if ( !ret )
        return ret;
    uchar *buf = m_file->map(0, m_file->size());
    m_impl = reinterpret_cast<struct ImageImpl*>(buf);
    m_notbacked=false;
    return true;
}

void Image::create(long rows, long columns,
                   Image::ColorSpace colorSpace,
                   Image::PixelSize pixelSize)
{
    size_t size = sizeof(*m_impl) +
            pixelSize *
            Image::ColorSpaceComponents(colorSpace) *
            rows * columns;
    m_impl = reinterpret_cast<ImageImpl*>(new char[size]);
    m_impl->colorSpace = colorSpace;
    m_impl->pixelSize = pixelSize;
    m_impl->rows = rows;
    m_impl->columns = columns;
    m_notbacked = true;
}

void Image::createAlike(const Image *image)
{
    create(image->m_impl->rows,
           image->m_impl->columns,
           image->m_impl->colorSpace,
           image->m_impl->pixelSize);
}

uchar *
Image::getRawPixels()
{
    if ( !m_impl )
        return NULL;
    return m_impl->pixels;
}


bool Image::save()
{
    bool ret;
    if (!m_notbacked)
        return false;
    ret = m_file->open(QFile::WriteOnly);
    if (!ret)
        return false;
    ret = m_file->write(reinterpret_cast<char*>(m_impl),
                        sizeof(*m_impl) +
                        m_impl->pixelSize *
                        Image::ColorSpaceComponents(m_impl->colorSpace) *
                        m_impl->rows * m_impl->columns);
    close();
    return true;
}

long Image::width()
{
    return m_impl->columns;
}

long Image::height()
{
    return m_impl->rows;
}

Image::PixelSize Image::getPixelSize()
{
    return m_impl->pixelSize;
}

Image::ColorSpace Image::getColorSpace()
{
    return m_impl->colorSpace;
}
QString Image::getFilename() const
{
    return m_filename;
}


bool Image::remove()
{
    if (m_owner)
        return m_file->remove();
    return true;
}
