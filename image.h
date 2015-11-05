#ifndef IMAGE_H
#define IMAGE_H
#include <QObject>


Q_STATIC_ASSERT(MAGICKCORE_HDRI_ENABLE == 0);
Q_STATIC_ASSERT(MAGICKCORE_QUANTUM_DEPTH == 16);

struct ImageImpl;
class QFile;

class Image
{
public:
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef float fp32;
    typedef double fp64;
    typedef enum {
        LinearMono,
        sMono,
        LinearRGB,
        sRGB,
        LinearLab,
        Lab
    } ColorSpace;
    typedef enum {
        Pixel_u8 = 1,
        Pixel_u16 = 2,
        Pixel_fp32 = 4,
        Pixel_fp64 = 8
    } PixelSize;

    static QString NewRandomName();
    static int ColorSpaceComponents(ColorSpace colorspace);

    Image(const QString& filename, bool owner=true);
    bool open();
    void close();
    bool remove();
    void create(long rows, long columns, ColorSpace colorSpace, PixelSize pixelSize);
    void createAlike(const Image *image);
    uchar *getRawPixels();
    bool save();
    long width();
    long height();
    PixelSize getPixelSize();
    ColorSpace getColorSpace();
    template<typename T> T* getPixelsMono() {
        if ( sizeof(T) != getPixelSize()) return NULL;
        if ( getColorSpace() > sMono ) return NULL;
        return reinterpret_cast<T*>(getRawPixels());
    }
    template<typename T>
    struct __attribute__((packed)) Triplet {
        Triplet(T rr, T gg, T bb) : r(rr), g(gg), b(bb) {}
        T r;
        T g;
        T b;
    };

    template<typename T> Triplet<T>* getPixelsTriplet() {
        if ( sizeof(T) != getPixelSize()) return NULL;
        if (getColorSpace() < LinearRGB ) return NULL;
        return reinterpret_cast<Triplet<T>*>(getRawPixels());
    }
    QString getFilename() const;

private:
    Q_DISABLE_COPY(Image);
    ImageImpl *m_impl;
    QFile *m_file;
    QString m_filename;
    bool m_notbacked;
    bool m_owner;
};

#endif // IMAGE_H
