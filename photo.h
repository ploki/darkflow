#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>

namespace Magick {
class Image;
class Blob;
}

/* macros defined in cc command line by pkg-config */
Q_STATIC_ASSERT(MAGICKCORE_HDRI_ENABLE == 0);
Q_STATIC_ASSERT(MAGICKCORE_QUANTUM_DEPTH == 16);

class Photo : public QObject
{
    Q_OBJECT
public:
    Photo(QObject *parent = 0);
    Photo(const Magick::Image *image, QObject *parent = 0);
    Photo(const Magick::Blob &blob, QObject *parent = 0);
    Photo(const Photo& photo);

    ~Photo();

    Photo& operator=(const Photo& photo);

    bool load(const QString& filename);
    bool save(const QString& filename, const QString &magick);

    void create(long width, long height);
    void createAlike(const Photo *photo);


    bool error() const;

    Magick::Image *image() const;

private:
    Magick::Image *m_image;
    bool m_error;

    void setError();
};

#endif // IMAGE_H
