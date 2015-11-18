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

Photo::Photo(Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(),
    m_curve(newCurve(gamma)),
    m_gamma(gamma),
    m_error(true),
    m_tags(),
    m_sequenceNumber(0)
{
}

Photo::Photo(const Blob &blob, Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(Magick::Image(blob)),
    m_curve(newCurve(gamma)),
    m_gamma(gamma),
    m_error(false),
    m_tags(),
    m_sequenceNumber(0)
{
}

Photo::Photo(const Magick::Image& image, Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(image),
    m_curve(newCurve(gamma)),
    m_gamma(gamma),
    m_error(false),
    m_tags(),
    m_sequenceNumber(0)
{
}

Photo::Photo(const Photo &photo) :
    QObject(photo.parent()),
    m_image(photo.m_image),
    m_curve(photo.m_curve),
    m_gamma(photo.m_gamma),
    m_error(false),
    m_tags(photo.m_tags),
    m_sequenceNumber(photo.m_sequenceNumber)
{
}

Photo::~Photo()
{
}

Photo &Photo::operator=(const Photo &photo)
{
    m_image = photo.m_image;
    m_curve = photo.m_curve;
    m_gamma = photo.m_gamma;
    m_tags = photo.m_tags;
    m_sequenceNumber = photo.m_sequenceNumber;
    m_error = false;
    return *this;
}

bool Photo::load(const QString &filename)
{
    QFile file(filename);
    if ( !file.open(QFile::ReadOnly) )
        return false;
    QByteArray data = file.readAll();
    try {
        Blob blob(data.data(), data.length());
        m_image = Image(blob);
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
    try {
        Blob blob;
        m_image.write(&blob, magick.toStdString());
        QFile file(filename);
        file.open(QFile::WriteOnly);
        if ( !file.write(reinterpret_cast<const char*>(blob.data()),blob.length()) ) {
            setError();
            return false;
        }
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
        return false;
    }
    return true;
}

void Photo::create(long width, long height)
{
    try {
        m_image = Magick::Image(Geometry(width,height),Color(0,0,0));
        m_image.quantizeColorSpace(Magick::RGBColorspace);
        m_error = false;
    }
    catch (std::exception *e) {
        qWarning(e->what());
        delete e;
        setError();
    }
}

void Photo::createAlike(const Photo& photo)
{
    create(photo.m_image.columns(), photo.m_image.rows());
}

bool Photo::error() const
{
    return m_error;
}

const Magick::Image& Photo::image() const
{
    return m_image;
}

Magick::Image& Photo::image()
{
    return m_image;
}

const Image &Photo::curve() const
{
    return m_curve;
}

Image &Photo::curve()
{
    return m_curve;
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
    m_image = Magick::Image();
}

Image Photo::newCurve(Photo::Gamma gamma)
{
    Image curve;
    curve.size("65536x1");
    curve.modifyImage();
    Pixels curve_cache(curve);
    PixelPacket *pixels = curve_cache.get(0,0,65536,1);
    for ( int i = 0 ; i < 65536 ; ++i )
        pixels[i].red = pixels[i].green = pixels[i].blue = i;
    switch(gamma) {
    default:
    case Linear:
        break;
    case Sqrt:
        iGamma(2,0).applyOnImage(curve);
        break;
    case IUT_BT_709:
        iGamma::BT709().applyOnImage(curve);
        break;
    case sRGB:
        iGamma::sRGB().applyOnImage(curve);
        break;
    }
    return curve;
}

static QPixmap convert(Magick::Image& image) {
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

QPixmap Photo::imageToPixmap(double gamma, double x0, double exposureBoost)
{
    Photo photo(*this);
    Exposure(exposureBoost).applyOn(photo);
    iGamma(gamma, x0).applyOn(photo);

    Magick::Image& image = photo.image();
    return convert(image);
}


#define SRGB_G 2.4L
#define SRGB_N 0.00304L

static double calcGamma(double v) {
        double a=-(SRGB_G-1.L)*pow(SRGB_N,(1.L/SRGB_G))/((SRGB_G-1.L)*pow(SRGB_N,(1.L/SRGB_G))-SRGB_G);
        double p = (a+1.L)*pow(SRGB_N,1.L/SRGB_G)/(SRGB_G*SRGB_N);
        if ( v < SRGB_N ) return v*p;
        else return (1.L+a)*pow(v,1.L/SRGB_G)-a;

}

#define PXL(x,y) pixels[(y)*512+(x)]
QPixmap Photo::curveToPixmap(Photo::CurveView cv)
{
    Magick::Image image("512x512", "black");
    Magick::Image curve(this->curve());
    Magick::Pixels image_cache(image);
    Magick::PixelPacket *pixels = image_cache.get(0,0, 512, 512);
    const bool zoneV_18=false;
    //Vertical lines (input)
    for ( int x = 32 ; x < 512 ; x+=32 ) {
        int c=QuantumRange/4;
        if ( x == 512 - 3*32 ) c*=2;
        //      if ( x == 512 - 9*32 ) c*=2;
        if ( x == 512 - 12*32 ) c*=1.5;
        for ( int y = 0 ; y < 512 ; y++ ) {
            PXL(x+(zoneV_18?16:0),y).red = c;
            PXL(x+(zoneV_18?16:0),y).green = c;
            PXL(x+(zoneV_18?16:0),y).blue = c;
        }
    }

    //Horizontal lines (output)
    for ( int il = 1; il < 16 ; ++il ) {
        int c=QuantumRange/4;
        if ( il == 3 ) c*=2;
        if ( il == 9 ) c*=2;   //practical limit
        if ( il == 12 ) c*=1.5; //theorical limit
        //double v=1.L/pow(2.L,16-il);
        int y=128;
        switch(cv) {
        case sRGB_Level: {
            double v=calcGamma(1.L/pow(2.L,il));
            y=v*512.L-1.L;
        }
            break;
        case sRGB_EV:
        case Log2:
            y = (16.L-il)*32.L-1.L;
            break;
        }
        if ( y < 0 ) y = 0;
        else if ( y > 511 ) y = 511;
        for ( int x = 0 ; x < 512 ; ++x ) {
            PXL(x,y).red =c;
            PXL(x,y).green =c;
            PXL(x,y).blue = c;
        }
    }
    //curve
    if ( cv == sRGB_EV )
        iGamma::reverse_sRGB().applyOnImage(curve);
    else if ( cv == Log2 )
        curve.gamma(1.L/2.2L);

    for ( int x=0 ; x < 65536 ; ++x ) {
        Magick::Color col = curve.pixelColor(x,0);
        double x0 = log(double(x+1)/65536.L)/log(2);
        // x0 compris entre 0 et -16
        int i = (16.L+x0)*32.L-1.L;
        int yr=0,yg=0,yb=0;
        switch ( cv ) {
        case sRGB_Level:
            yr=col.redQuantum()/128.L;
            yg=col.greenQuantum()/128.L;
            yb=col.blueQuantum()/128.L;
            break;
        case sRGB_EV:
        case Log2: {
            double  v = double(col.redQuantum()+1)/double(QuantumRange+1);
            v = log2(v);
            yr=  (16.L+v)*32.L-1.L;
            if ( yr < 0 ) yr = 0;
            else if ( yr > 511 ) yr = 511;
            v = double(col.greenQuantum()+1)/double(QuantumRange+1);
            v = log2(v);
            yg=  (16.L+v)*32.L-1.L;
            if ( yg < 0 ) yg = 0;
            else if ( yg > 511 ) yg = 511;
            v = double(col.blueQuantum()+1)/double(QuantumRange+1);
            v = log2(v);
            yb=  (16.L+v)*32.L-1.L;
            if ( yb < 0 ) yb = 0;
            else if ( yb > 511 ) yb = 511;
            break;
        }
        default://plop
            i=256;
        }
        if ( i < 0 ) i = 0;
        else if ( i > 511 ) i = 511;
        PXL(i,yr).red = QuantumRange;
        PXL(i,yg).green = QuantumRange;
        PXL(i,yb).blue = QuantumRange;
    }
    image_cache.sync();
    /*
             for ( int x=0 ; x < 512 ; ++x) {
             Magick::Color col = curve.pixelColor(x*128,0);
             int c = col.redQuantum();
                     image.pixelColor(x,
                             c/128,
                     Color(QuantumRange,QuantumRange,QuantumRange,0));
             }
    */
    image.flip();
    return convert(image);
}


void Photo::writeJPG(const QString &filename)
{
    Magick::Image image(m_image);
    image.modifyImage();
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



