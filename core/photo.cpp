#include "photo.h"
#include <QFile>
#include <QString>
#include <QPixmap>
#include <QRectF>
#include <Magick++.h>
#include <cmath>

#include <string>
#include <cstdio>

#include "ports.h"
#include "igamma.h"
#include "hdr.h"
#include "exposure.h"
#include "process.h"
#include "console.h"

using namespace Magick;

Photo::Photo(Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(),
    m_curve(newCurve(gamma)),
    m_status(Photo::Undefined),
    m_tags(),
    m_identity(Process::uuid()),
    m_sequenceNumber(0)
{
    setScale(gamma);
}

Photo::Photo(const Blob &blob, Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(blob),
    m_curve(newCurve(gamma)),
    m_status(Photo::Complete),
    m_tags(),
    m_identity(Process::uuid()),
    m_sequenceNumber(0)
{
    setScale(gamma);
}

Photo::Photo(const Magick::Image& image, Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(image),
    m_curve(newCurve(gamma)),
    m_status(Photo::Complete),
    m_tags(),
    m_identity(Process::uuid()),
    m_sequenceNumber(0)
{
    setScale(gamma);
}

Photo::Photo(const Photo &photo) :
    QObject(photo.parent()),
    m_image(photo.m_image),
    m_curve(photo.m_curve),
    m_status(photo.m_status),
    m_tags(photo.m_tags),
    m_identity(photo.m_identity),
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
    m_tags = photo.m_tags;
    m_identity = photo.m_identity;
    m_sequenceNumber = photo.m_sequenceNumber;
    m_status = photo.m_status;
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
        m_status = Photo::Complete;
    }
    catch (std::exception& e) {
        dflError(e.what());
        setUndefined();
        return false;
    }
    m_tags.clear();
    m_tags[TAG_NAME] = filename;
    setIdentity(filename);
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
            setUndefined();
            return false;
        }
    }
    catch (std::exception& e) {
        dflError(e.what());
        setUndefined();
        return false;
    }
    return true;
}

void Photo::createImage(long width, long height)
{
    try {
        m_image = Magick::Image(Geometry(width,height),Color(0,0,0));
        m_image.quantizeColorSpace(Magick::RGBColorspace);
        m_status = Complete;
    }
    catch (std::exception &e) {
        dflError(e.what());
        setUndefined();
    }
}

void Photo::createImageAlike(const Photo& photo)
{
    createImage(photo.m_image.columns(), photo.m_image.rows());
}

QVector<qreal> Photo::pixelColor(unsigned x, unsigned y)
{
    QVector<qreal> rgb(3);
    if ( x >= m_image.columns() ||
         y >= m_image.rows() )
        return rgb;
    try {
        Magick::Pixels cache(m_image);
        const Magick::PixelPacket *pixel = cache.getConst(x,y,1,1);
        if (pixel) {
            if ( getScale() == HDR ) {
                rgb[0] = fromHDR(pixel->red);
                rgb[1] = fromHDR(pixel->green);
                rgb[2] = fromHDR(pixel->blue);
            }
            else {
                rgb[0] = pixel->red;
                rgb[1] = pixel->green;
                rgb[2] = pixel->blue;
            }
        }
    }
    catch (std::exception &e) {
        dflError("Unable to get pixel color: %s", e.what());
    }

    return rgb;
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


Image Photo::newCurve(Photo::Gamma gamma)
{
    Image curve;
    curve.size("65536x1");
    curve.modifyImage();
    Pixels curve_cache(curve);
    PixelPacket *pixels = curve_cache.get(0,0,65536,1);
    for ( int i = 0 ; i < 65536 ; ++i )
        pixels[i].red = pixels[i].green = pixels[i].blue = i;
    curve_cache.sync();
    switch(gamma) {
    default:
    case NonLinear:
        dflWarning("Unable to create curve for NonLinear");
        break;
    case HDR: {
        ::HDR(false).applyOnImage(curve,false);
        break;
    }
    case Linear:
        break;
    case Sqrt:
        iGamma(2,0).applyOnImage(curve, false);
        break;
    case IUT_BT_709:
        iGamma::BT709().applyOnImage(curve, false);
        break;
    case sRGB:
        iGamma::sRGB().applyOnImage(curve, false);
        break;
    }
    return curve;
}

static QPixmap convert(Magick::Image& image) {
    int h = image.rows(),
            w = image.columns();
    Magick::Pixels pixel_cache(image);
    unsigned char pgm_header[256];
    int header_size = snprintf((char*)pgm_header, sizeof pgm_header,
             "P6\n%d %d\n%d\n",w,h,255);
    unsigned char *buf = (unsigned char *)malloc(header_size+w*h*3*1);
    memcpy(buf, pgm_header, header_size);
    bool error=false;
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        const Magick::PixelPacket *pixels = pixel_cache.getConst(0,y,w,1);
        if ( error || !pixels ) {
            if ( !error )
                dflError("PixelPacket is NULL");
            error = true;
            continue;
        }
        for ( int x = 0 ; x < w ; ++x ) {
            buf[header_size+y*w*3+x*3+0]=pixels[x].red/256;
            buf[header_size+y*w*3+x*3+1]=pixels[x].green/256;
            buf[header_size+y*w*3+x*3+2]=pixels[x].blue/256;
        }
    }
  QPixmap pix(w,h);
   bool ret = pix.loadFromData(buf, header_size+w*h*3*1, "PPM", Qt::AutoColor|Qt::AvoidDither);
   if ( !ret )
       dflError("pixmap conversion failed");
   free(buf);
   return pix;
}

QPixmap Photo::imageToPixmap(double gamma, double x0, double exposureBoost)
{
    Q_UNUSED(gamma);
    Q_UNUSED(x0);
    Q_ASSERT( m_status == Complete );
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
    Q_ASSERT( m_status == Complete );
    Magick::Image image("512x512", "black");
    Magick::Image curve(this->curve());
    image.modifyImage();
    Magick::Pixels image_cache(image);

    if ( getScale() == HDR ) {
        ::HDR hdrRevert(true);
        hdrRevert.applyOnImage(curve, true);
    }

    Magick::PixelPacket *pixels = image_cache.get(0,0, 512, 512);
    iGamma& g = iGamma::sRGB();
    const bool zoneV_18=false;
    //Vertical lines (input)
    for ( int x = 32 ; x < 512 ; x+=32 ) {
#if 0
        quantum_t c=QuantumRange/4;
        if ( x == 512 - 3*32 ) c*=2;
        //      if ( x == 512 - 9*32 ) c*=2;
        if ( x == 512 - 12*32 ) c*=1.5;
#else
        quantum_t c = g.applyOnQuantum(pow(2,double(x)/32), false);
#endif
        for ( int y = 0 ; y < 512 ; y++ ) {
            PXL(x+(zoneV_18?16:0),y).red = c;
            PXL(x+(zoneV_18?16:0),y).green = c;
            PXL(x+(zoneV_18?16:0),y).blue = c;
        }
    }

    //Horizontal lines (output)
    for ( int il = 1; il < 16 ; ++il ) {
#if 0
        int c=QuantumRange/4;
        if ( il == 3 ) c*=2;
        if ( il == 9 ) c*=2;   //practical limit
        if ( il == 12 ) c*=1.5; //theorical limit
#else
        quantum_t c = g.applyOnQuantum(pow(2,16-il), false);
#endif
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
        iGamma::reverse_sRGB().applyOnImage(curve, false);
    else if ( cv == Log2 )
        curve.gamma(1.L/2.2L);

    Magick::Pixels curve_cache(curve);
    const Magick::PixelPacket *curve_pixels = curve_cache.getConst(0,0,65536,1);
    for ( int x=0 ; x < 65536 ; ++x ) {
        double x0 = log(double(x+1)/65536.L)/log(2);
        // x0 compris entre 0 et -16
        int i = (16.L+x0)*32.L-1.L;
        int yr=0,yg=0,yb=0;
        switch ( cv ) {
        case sRGB_Level:
            yr=curve_pixels[x].red/128.L;
            yg=curve_pixels[x].green/128.L;
            yb=curve_pixels[x].blue/128.L;
            break;
        case sRGB_EV:
        case Log2: {
            double  v = double(curve_pixels[x].red+1)/double(QuantumRange+1);
            v = log2(v);
            yr=  (16.L+v)*32.L-1.L;
            if ( yr < 0 ) yr = 0;
            else if ( yr > 511 ) yr = 511;
            v = double(curve_pixels[x].green+1)/double(QuantumRange+1);
            v = log2(v);
            yg=  (16.L+v)*32.L-1.L;
            if ( yg < 0 ) yg = 0;
            else if ( yg > 511 ) yg = 511;
            v = double(curve_pixels[x].blue+1)/double(QuantumRange+1);
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

QPixmap Photo::histogramToPixmap(Photo::HistogramScale scale, Photo::HistogramGeometry geometry)
{
    Q_ASSERT( m_status == Complete );
    Magick::Image image;
    Magick::Image photo = this->image();

    /*
    if ( getScale() == HDR ) {
        ::HDR hdrRevert(true);
        hdrRevert.applyOnImage(photo, true);
    }
   */

    bool adt = (geometry == HistogramLines);
    bool hlog = (scale == HistogramLogarithmic);
    int x,y;

    int w = photo.columns(),
            h = photo.rows();

    const int range = 512;
    unsigned long histo[range][3]={{0}};
    unsigned long maxi=0;

    Magick::Pixels photo_cache(photo);
    photo.modifyImage();
#pragma omp parallel for
    for ( y = 0 ; y < h ; ++y ) {
        const Magick::PixelPacket *pixels = photo_cache.getConst(0,y,w,1);
        if (!pixels ) continue;
        for ( x = 0 ; x < w ; ++x ) {
            int r=(range-1)*pixels[x].red/QuantumRange;
            int g=(range-1)*pixels[x].green/QuantumRange;
            int b=(range-1)*pixels[x].blue/QuantumRange;
            Q_ASSERT(r < range);
            Q_ASSERT(g < range);
            Q_ASSERT(b < range);
            atomic_incr(&histo[r][0]);
            atomic_incr(&histo[g][1]);
            atomic_incr(&histo[b][2]);
            if ( r > 2 && r < (range-1) && histo[r][0] > maxi ) maxi=histo[r][0];
            if ( g > 2 && g < (range-1) && histo[g][1] > maxi ) maxi=histo[g][1];
            if ( b > 2 && b < (range-1) && histo[b][2] > maxi ) maxi=histo[b][2];
        }
        photo_cache.sync();
    }
    image = Image( "512x512" , "black" );
    image.modifyImage();
    Magick::Pixels image_cache(image);
    Magick::PixelPacket *pixels = image_cache.get(0, 0, range, range);
    for ( x=0 ; x < range ; ++x ) {
        quantum_t qr;
        quantum_t qg;
        quantum_t qb;
        if ( hlog )
        {
            qg = (range-1)*log(histo[x][1])/log(maxi);
            qr = (range-1)*log(histo[x][0])/log(maxi);
            qb = (range-1)*log(histo[x][2])/log(maxi);
        }
        else
        {
            qg = (range-1)*(double)histo[x][1]/(double)maxi;
            qr = (range-1)*(double)histo[x][0]/(double)maxi;
            qb = (range-1)*(double)histo[x][2]/(double)maxi;
        }

        if ( qr > (range-1) ) qr = (range-1);
        if ( qg > (range-1) ) qg = (range-1);
        if ( qb > (range-1) ) qb = (range-1);

        if ( qr == 0 && histo[x][0] > 0 ) qr=1;
        if ( qg == 0 && histo[x][1] > 0 ) qg=1;
        if ( qb == 0 && histo[x][2] > 0 ) qb=1;
        for (int i = 0 ; i < qr ; ++i )
        {
            PXL(x, (range-1)-i).red = QuantumRange;
        }
        for (int i = 0 ; i < qg ; ++i )
        {
            PXL(x, (range-1)-i).green = QuantumRange;
        }
        for (int i = 0 ; i < qb ; ++i )
        {
            PXL(x, (range-1)-i).blue = QuantumRange;
        }
    }
    image_cache.sync();
    if ( adt )
        image.adaptiveThreshold(4,4,0.);
    {
        image.modifyImage();
        Magick::Pixels image_cache(image);
        Magick::PixelPacket *pixels = image_cache.get(0, 0, range, range);
        iGamma& g = iGamma::sRGB();
        int marks_non_linear[] = { g.applyOnQuantum(1<<15, false)/128,
                                   g.applyOnQuantum(1<<14, false)/128,
                                   g.applyOnQuantum(1<<13, false)/128,
                                   g.applyOnQuantum(1<<12, false)/128,
                                   g.applyOnQuantum(1<<11, false)/128,
                                   g.applyOnQuantum(1<<10, false)/128,
                                   g.applyOnQuantum(1<<9, false)/128,
                                   g.applyOnQuantum(1<<8, false)/128,
                                   g.applyOnQuantum(1<<7, false)/128,
                                   g.applyOnQuantum(1<<6, false)/128,
                                   0 };
        int marks_linear[] = {
            (1<<15)/  128,
            (1<<14)/  128,
            (1<<13)/  128,
            (1<<12)/  128,
            (1<<11)/  128,
            (1<<10)/  128,
            (1<<9)/  128,
            (1<<8)/  128,
            0
        };
        int marks_hdr[] = {
            32*1,
            32*2,
            32*3,
            32*4,
            32*5,
            32*6,
            32*7,
            32*8,
            32*9,
            32*10,
            32*11,
            32*12,
            32*13,
            32*14,
            32*15,
            0
        };
        int *marks;
        enum { C_hdr,
               C_linear,
               C_nonlinear } c;

        if ( getScale() == HDR ) {
            marks=marks_hdr;
            c = C_hdr;
        }
        else if ( getScale() == Linear ) {
            marks=marks_linear;
            c = C_linear;
        }
        else if ( getScale() == NonLinear ) {
            marks=marks_non_linear;
            c = C_nonlinear;
        }
        else {
            dflError("unknown scale");
            marks=marks_linear;
            c = C_linear;
        }
        for ( int i = 0 ; marks[i] != 0 ; ++i ) {
            x = marks[i];
            for (int y = 0 ; y < range ; ++ y ) {
                quantum_t color;
                switch (c) {
                case C_nonlinear:
                    color = x*128;
                    break;
                case C_linear:
                    color = iGamma::sRGB().applyOnQuantum(x*128, false);
                    break;
                case C_hdr:
                    color = iGamma::sRGB().applyOnQuantum(x*128, true);
                }

                if ( y < 12 || y >= range-12 ) {
                    /* add some light on top and bottom of the frame*/
                    color = clamp( color +
                                   QuantumRange/(y?y:1) +
                                   QuantumRange/((range-y))
                                   );
                }
                PXL(x,y).red = color;
                PXL(x,y).green = color;
                PXL(x,y).blue = color;
            }
        }
        image_cache.sync();
    }
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
    if ( m_sequenceNumber == other.m_sequenceNumber )
        return (m_identity < other.m_identity);
    return (m_sequenceNumber < other.m_sequenceNumber);
}

QString Photo::getIdentity() const
{
    return m_identity;
}

void Photo::setIdentity(const QString &identity)
{
    m_identity = identity;
    if ( m_status == Undefined )
        m_status = Identified;
}

void Photo::setUndefined()
{
    m_status = Undefined;
    m_image = Magick::Image();
}

void Photo::setComplete()
{
    m_status = Complete;
}

bool Photo::isUndefined() const
{
    return m_status == Undefined;
}

bool Photo::isIdentified() const
{
    return m_status == Identified;
}

bool Photo::isComplete() const
{
    return m_status == Complete;
}

QVector<QPointF> Photo::getPoints() const
{
    QVector<QPointF> vec;
    QStringList points = getTag(TAG_POINTS).split(';');
    if ( points.count() == 1 && points[0].count() == 0 )
        return vec;
    foreach(QString point, points) {
        if ( point.count() == 0 )
            continue;
        QStringList coords = point.split(',');
        if ( coords.count() != 2 ) {
            dflError("Photo: Invalid numbers in " TAG_POINTS);
            continue;
        }
        vec.push_back(QPointF(coords[0].toDouble(), coords[1].toDouble()));
    }
    return vec;
}

void Photo::setPoints(const QVector<QPointF>& vec)
{
    QString points;
    foreach(QPointF p, vec) {
        if ( !points.isEmpty() )
            points+=";";
        points+=QString::number(p.x())+","+QString::number(p.y());
    }
    setTag(TAG_POINTS, points);
}

QRectF Photo::getROI() const
{
    QString roiTag = getTag(TAG_ROI);
    if ( !roiTag.isEmpty() ) {
        QStringList coord = roiTag.split(',');
        if ( coord.size() == 4 ) {
            qreal x1 = coord[0].toDouble();
            qreal y1 = coord[1].toDouble();
            qreal x2 = coord[2].toDouble();
            qreal y2 = coord[3].toDouble();
            if ( x1 < 0 ) x1 = 0;
            if ( y1 < 0 ) y1 = 0;
            if ( x2 < 0 ) x2 = 0;
            if ( y2 < 0 ) y2 = 0;
            if ( x1 > image().columns() ) x1=image().columns();
            if ( y1 > image().rows() ) y1=image().rows();
            if ( x2 > image().columns() ) x2=image().columns();
            if ( y2 > image().rows() ) y2=image().rows();
            qreal x=x1,y=y1,w=x2-x1,h=y2-y1;
            if ( x1 > x2 ) {
                x=x2; w=-w;
            }
            if ( y1 > y2 ) {
                y=y2; h=-h;
            }
            //dflDebug("x1:%f, y1:%f, x2:%f, y2:%f",x1,y1,x2,y2);
            //dflDebug("x:%f, y:%f, w:%f, h:%f",x,y,w,h);
            return QRectF(x,y,w,h);
        }
    }
    return QRectF();
}

void Photo::setROI(const QRectF &rect)
{
    QString roi = QString::number(rect.x()) + "," +
            QString::number(rect.y()) + "," +
            QString::number(rect.x()+rect.width()) + "," +
            QString::number(rect.y()+rect.height());
    setTag(TAG_ROI,roi);
}

void Photo::setScale(Photo::Gamma gamma)
{
    if ( gamma & Linear ) {
        setTag(TAG_SCALE, TAG_SCALE_LINEAR);
    } else if ( gamma & NonLinear ) {
        setTag(TAG_SCALE, TAG_SCALE_NONLINEAR);
    } else if ( gamma & HDR ) {
        setTag(TAG_SCALE, TAG_SCALE_HDR);
    }
}

Photo::Gamma Photo::getScale() const
{
    QString scale = getTag(TAG_SCALE);
    if  ( scale == TAG_SCALE_LINEAR )
        return Linear;
    else if ( scale == TAG_SCALE_NONLINEAR )
        return NonLinear;
    else if ( scale == TAG_SCALE_HDR )
        return HDR;
    else {
        dflWarning("Unknown photo scale");
        return Linear;
    }
}



