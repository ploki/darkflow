/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include "photo.h"
#include <QFile>
#include <QString>
#include <QPixmap>
#include <QElapsedTimer>
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
#include "preferences.h"

using Magick::Quantum;

Photo::Photo(Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(),
    m_curve(newCurve()),
    m_status(Photo::Undefined),
    m_tags(),
    m_identity(Process::uuid()),
    m_sequenceNumber(0)
{
    setScale(gamma);
}

Photo::Photo(const Magick::Blob &blob, Photo::Gamma gamma, QObject *parent) :
    QObject(parent),
    m_image(blob),
    m_curve(newCurve()),
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
    m_curve(newCurve()),
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
        Magick::Blob blob(data.data(), data.length());
        m_image = Magick::Image(blob);
        m_status = Photo::Complete;
    }
    catch (std::exception& e) {
        dflCritical("%s", e.what());
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
        Magick::Blob blob;
        m_image.write(&blob, magick.toStdString());
        QFile file(filename);
        file.open(QFile::WriteOnly);
        if ( !file.write(reinterpret_cast<const char*>(blob.data()),blob.length()) ) {
            setUndefined();
            return false;
        }
    }
    catch (std::exception& e) {
        dflCritical("%s", e.what());
        setUndefined();
        return false;
    }
    return true;
}

void Photo::createImage(long width, long height)
{
    try {
        m_image = Magick::Image(Magick::Geometry(width,height),Magick::Color(0,0,0));
        m_image.quantizeColorSpace(Magick::RGBColorspace);
        m_status = Complete;
    }
    catch (std::exception &e) {
        dflCritical("%s", e.what());
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
        Ordinary::Pixels cache(m_image);
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
        dflCritical(tr("Unable to get pixel color: %0").arg(e.what()));
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

const Magick::Image &Photo::curve() const
{
    return m_curve;
}

Magick::Image &Photo::curve()
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


Magick::Image Photo::newCurve()
{
    Magick::Image curve(Magick::Geometry(65536, 1), Magick::Color(0, 0, 0));
    Ordinary::Pixels curve_cache(curve);
    Magick::PixelPacket *pixels = curve_cache.get(0,0,65536,1);
    for ( int i = 0 ; i < 65536 ; ++i )
        pixels[i].red = pixels[i].green = pixels[i].blue = i;
    curve_cache.sync();
    return curve;
}

STFLut::STFLut(bool isHDR,
               double encGamma, double encX0,
               double displayGamma, double displayX0,
               double exposureBoost)
{
    iGamma ig(encGamma, encX0, true);
    Exposure ev(exposureBoost);
    iGamma og(displayGamma, displayX0, false);
    for (int i = 0; i < 1+QuantumRange; ++i) {
        quantum_t decodedQ = i;
        if ((!isHDR || encGamma != 1) && displayGamma != 1) {
            decodedQ = clamp((int)ig.applyOnQuantum(i, false));
        }
        quantum_t brightenedQ = clamp((int)ev.applyOnQuantum(decodedQ, isHDR));
        quantum_t displayedQ = clamp((int)og.applyOnQuantum(brightenedQ, isHDR));
        m_lut[i] = clamp(quantum_t(DF_ROUND(qreal(displayedQ)/255)), 0, 255);
    }
}
unsigned char STFLut::operator[](int i) {
    return m_lut[i];
}

static QPixmap convertToPixmap(STFLut& stf, Magick::Image& image) {
    int h = image.rows(),
            w = image.columns();
    std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
    unsigned char pgm_header[256];
    int header_size = snprintf((char*)pgm_header, sizeof pgm_header,
             "P6\n%d %d\n%d\n",w,h,255);
    unsigned char *buf = (unsigned char *)malloc(header_size+w*h*3*1);
    memcpy(buf, pgm_header, header_size);
    dfl_block bool error=false;
    dfl_parallel_for(y, 0, h, 4, (image), {
        const Magick::PixelPacket *pixels = pixel_cache->getConst(0,y,w,1);
        if ( error || !pixels ) {
            if ( !error )
                dflCritical(DF_NULL_PIXELS);
            error = true;
            continue;
        }
        for ( int x = 0 ; x < w ; ++x ) {
            int t = header_size + 3*(y*w + x);
            buf[t+0]=stf[pixels[x].red];
            buf[t+1]=stf[pixels[x].green];
            buf[t+2]=stf[pixels[x].blue];
        }
    });
  QPixmap pix(w,h);
   bool ret = pix.loadFromData(buf, header_size+w*h*3*1, "PPM", Qt::AutoColor|Qt::AvoidDither);
   if ( !ret )
       dflError(Photo::tr("Pixmap conversion failed"));
   free(buf);
   return pix;
}

QPixmap Photo::imageToPixmap(STFLut& stf)
{
    Q_ASSERT( m_status == Complete );
    //Photo photo(*this);
    return convertToPixmap(stf, image());
}

#define PXL(x,y) pixels[(y)*512+(x)]
QPixmap Photo::curveToPixmap(Photo::CurveView cv)
{
    Q_ASSERT( m_status == Complete );
    Magick::Image image(Magick::Geometry(512,512),Magick::Color(0,0,0));
    Magick::Image curve(this->curve());
    Ordinary::Pixels image_cache(image);


    Magick::PixelPacket *pixels = image_cache.get(0,0, 512, 512);
    if ( NULL == pixels ) {
        dflError(DF_NULL_PIXELS);
        return QPixmap();
    }

    //g is used to convert horz and vert lines to screen colors
    iGamma& g = iGamma::sRGB();

    //Vertical lines (input)
    for ( int x = 32 ; x < 512 ; x+=32 ) {
        quantum_t c = g.applyOnQuantum(pow(2,double(x)/32), false);
        for ( int y = 0 ; y < 512 ; y++ ) {
            PXL(x, y).red = c;
            PXL(x, y).green = c;
            PXL(x, y).blue = c;
        }
    }

    //Horizontal lines (output)
    for ( int il = 1; il < 16 ; ++il ) {
        quantum_t c = g.applyOnQuantum(pow(2,16-il), false);
        int y;
        switch(cv) {
        case sRGB_Level:
            y = double(c)/128.;
            break;
        case sRGB_EV:
        case Log2:
        default:
            y = ((16. - il) * 32.) - 1.;
            break;
        }
        y = clamp(y, 0, 511);
        for ( int x = 0 ; x < 512 ; ++x ) {
            PXL(x,y).red =c;
            PXL(x,y).green =c;
            PXL(x,y).blue = c;
        }
    }


    //curve

    /*
    if ( getScale() == HDR ) {
        ::HDR hdrRevert(true);
        hdrRevert.applyOnImage(curve, true);
    }

    if ( cv == sRGB_EV )
        iGamma::reverse_sRGB().applyOnImage(curve, false);
    else if ( cv == Log2 )
        curve.gamma(1.L/2.2L);
    */

    // rg => sRGG^gamma = linear
    // g => srgb = linear^{1/gamma}

    iGamma& rg = iGamma::reverse_sRGB();
    Ordinary::Pixels curve_cache(curve);
    const Magick::PixelPacket *curve_pixels = curve_cache.getConst(0,0,65536,1);
    for ( int x=1 ; x < 512 ; ++x ) {
        // x: pixel idx. 32px = 1IL, EV logscale,
        // il: EV value for x
        qreal il = double(x)/32;
        // lx: linear ADU corresponding to col[x]
        qreal lx = pow(2, il);
        int xx = lx;
        qreal o_r, o_g, o_b;
        if (getScale() == HDR) {
              //HDR buffer
              // idx domain, hdr values from 0 to 65535
              // buffer domain, hdr values from 0 to 65535
            o_r = fromHDR(curve_pixels[int(toHDR(lx))].red);
            o_g = fromHDR(curve_pixels[int(toHDR(lx))].green);
            o_b = fromHDR(curve_pixels[int(toHDR(lx))].blue);
        } else if (getScale() == Linear) {
            o_r = curve_pixels[xx].red;
            o_g = curve_pixels[xx].green;
            o_b = curve_pixels[xx].blue;
        } else {
            //assume sRGB target
            o_r = rg.applyOnQuantum(curve_pixels[xx].red, false);
            o_g = rg.applyOnQuantum(curve_pixels[xx].green, false);
            o_b = rg.applyOnQuantum(curve_pixels[xx].blue, false);
        }
        int yr, yg, yb;
        switch (cv) {
        case sRGB_Level:
            //yr = clamp(int(rg.applyOnQuantum(o_r, false)/128.),0,511);
            //yg = clamp(int(rg.applyOnQuantum(o_g, false)/128.),0,511);
            //yb = clamp(int(rg.applyOnQuantum(o_b, false)/128.),0,511);
            yr = clamp(int(g.applyOnQuantum(o_r, false)/128.),0,511);
            yg = clamp(int(g.applyOnQuantum(o_g, false)/128.),0,511);
            yb = clamp(int(g.applyOnQuantum(o_b, false)/128.),0,511);
            break;
        case sRGB_EV:
            yr = clamp(int( 32.*(16+log(
                                     qreal(
                                         rg.applyOnQuantum(g.applyOnQuantum(o_r, false),false)
                                         )/QuantumRange)
                                 /log(2))),0,511);
            yg = clamp(int( 32.*(16+log(
                                     qreal(
                                         rg.applyOnQuantum(g.applyOnQuantum(o_g, false),false)
                                         )/QuantumRange)
                                 /log(2))),0,511);
            yb = clamp(int( 32.*(16+log(
                                     qreal(
                                         rg.applyOnQuantum(g.applyOnQuantum(o_b, false), false)
                                         )/QuantumRange)
                                 /log(2))),0,511);
            break;
        case Log2: default:
            yr = clamp(int( 32.*(16+log(o_r/QuantumRange)/log(2))    ),0,511);
            yg = clamp(int( 32.*(16+log(o_g/QuantumRange)/log(2))    ),0,511);
            yb = clamp(int( 32.*(16+log(o_b/QuantumRange)/log(2))    ),0,511);
            break;
        }

        PXL(x, yr).red = QuantumRange;
        PXL(x, yg).green = QuantumRange;
        PXL(x, yb).blue = QuantumRange;
        /*
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
        */

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
    STFLut stf(false, 1., 0,  1., 0, 1.);
    return convertToPixmap(stf, image);
}
static bool ulrevcmp(unsigned long lhs, unsigned long rhs)
{
    return lhs > rhs;
}
QPixmap Photo::histogramToPixmap(Photo::HistogramScale scale, Photo::HistogramGeometry geometry)
{
    Q_ASSERT( m_status == Complete );
    Magick::Image & photo = this->image();

    /*
    if ( getScale() == HDR ) {
        ::HDR hdrRevert(true);
        hdrRevert.applyOnImage(photo, true);
    }
   */

    bool adt = (geometry == HistogramLines);
    bool hlog = (scale == HistogramLogarithmic);
    int x;

    int w = photo.columns(),
            h = photo.rows();

    const int range = 512;
    dfl_block_array(unsigned long, histo, range*3);
    dfl_block unsigned long maxi=1;

    {
        std::shared_ptr<Ordinary::Pixels> photo_cache(new Ordinary::Pixels(photo));
        dfl_parallel_for(y, 0, h, 4, (photo), {
            const Magick::PixelPacket *pixels = photo_cache->getConst(0,y,w,1);
            if (!pixels ) continue;
            for ( int x = 0 ; x < w ; ++x ) {
                int r=(range-1)*pixels[x].red/QuantumRange;
                int g=(range-1)*pixels[x].green/QuantumRange;
                int b=(range-1)*pixels[x].blue/QuantumRange;
                Q_ASSERT(r < range);
                Q_ASSERT(g < range);
                Q_ASSERT(b < range);
                atomic_incr(&histo[r+0*range]);
                atomic_incr(&histo[g+1*range]);
                atomic_incr(&histo[b+2*range]);
            }
        });
    }
    for(int c = 0; c < 3; ++c) {
        unsigned long hc[range];
        memcpy(&hc[0],&histo[c*range], sizeof(hc));
        qSort(&hc[0], &hc[range], ulrevcmp);
        if ( hc[2] > maxi ) {
            //let clip the outliers at both ends if the exist.
            //if there is no outliers and the histogram has a quite sharp spike
            //it will be cliped, not a practical problem
            maxi = hc[2];
        }

    }
    maxi = int(1.1*maxi);

    Magick::Image image( Magick::Geometry(512,512) , Magick::Color(0,0,0) );
    {
        Ordinary::Pixels image_cache(image);
        Magick::PixelPacket *pixels = image_cache.get(0, 0, range, range);
        for ( x=0 ; x < range ; ++x ) {
            quantum_t qr;
            quantum_t qg;
            quantum_t qb;
            if ( hlog )
            {
                qg = (range-1)*log(histo[x+1*range])/log(maxi);
                qr = (range-1)*log(histo[x+0*range])/log(maxi);
                qb = (range-1)*log(histo[x+2*range])/log(maxi);
            }
            else
            {
                qg = (range-1)*(double)histo[x+1*range]/(double)maxi;
                qr = (range-1)*(double)histo[x+0*range]/(double)maxi;
                qb = (range-1)*(double)histo[x+2*range]/(double)maxi;
            }

            if ( qr > (range-1) ) qr = (range-1);
            if ( qg > (range-1) ) qg = (range-1);
            if ( qb > (range-1) ) qb = (range-1);

            if ( qr == 0 && histo[x+0*range] > 0 ) qr=1;
            if ( qg == 0 && histo[x+1*range] > 0 ) qg=1;
            if ( qb == 0 && histo[x+2*range] > 0 ) qb=1;
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
    }
    if ( adt )
        image.adaptiveThreshold(4,4,0.);
    {
        Magick::Image srcImage(image);
        ResetImage(image);
        Ordinary::Pixels src_cache(srcImage);
        Ordinary::Pixels image_cache(image);
        Magick::PixelPacket *pixels = image_cache.get(0, 0, range, range);
        const Magick::PixelPacket *src = src_cache.get(0, 0, range, range);
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
        dfl_parallel_for(y, 0, range, 4, (srcImage, image), {
            for ( int x = 0 ; x < range ; ++x ) {
                PXL(x,y).red = src[y*range+x].red;
                PXL(x,y).green = src[y*range+x].green;
                PXL(x,y).blue = src[y*range+x].blue;
            }
        });
        for ( int i = 0 ; marks[i] != 0 ; ++i ) {
            x = marks[i];
            for (int y = 0 ; y < range ; ++ y ) {
                quantum_t color = 0;
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
    STFLut stf(false, 1., 0, 1., 0., 1.);
    return convertToPixmap(stf, image);
}


void Photo::writeJPG(const QString &filename)
{
    Magick::Image image(m_image);
    image.magick("JPG");
    Magick::Blob blob;
    image.write(&blob);
    QFile f(filename);
    f.open(QFile::WriteOnly);
    f.write((char*)blob.data(), blob.length());
}

bool Photo::saveImage(const QString &filename, const QString &magick, double gamma, double x0, double exposureBoost)
{
    Q_ASSERT( m_status == Complete );
    Photo photo(*this);
    Exposure(exposureBoost).applyOn(photo);
    iGamma(gamma, x0).applyOn(photo);
    try {
        Magick::Image& image = photo.image();
        Q_UNUSED(magick);
        //image.magick(magick.toStdString());
        image.fileName(filename.toStdString());
        Magick::Blob blob;
        image.write(&blob);
        QFile f(filename);
        bool success;
        success = f.open(QFile::WriteOnly);
        if ( !success ) {
            dflError(tr("Could not open file %0").arg(filename));
            return false;
        }
        quint64 l = f.write((char*)blob.data(), blob.length());
        if ( l != blob.length() ) {
            dflError(tr("Could not save file %0").arg(filename));
            return false;
        }
    }
    catch (std::exception &e) {
        dflError("%s", e.what());
        return false;
    }
    return true;
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
            dflError(tr("Photo: Invalid numbers in %0").arg(TAG_POINTS));
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
    switch(gamma) {
    default:
    case NonLinear:
        dflCritical(tr("Not expected to set an unspecified Non-linear gamma"));
        break;
    case HDR:
        // at the beginning, curve is the identity, the curve[i:HDR]=i:HDR,
        // thel let be curve[i]=i
        //::HDR(false).applyOnImage(m_curve,false);
        break;
    case Linear:
        break;
    case Sqrt:
        iGamma(2,0).applyOnImage(m_curve, false);
        break;
    case IUT_BT_709:
        iGamma::BT709().applyOnImage(m_curve, false);
        break;
    case sRGB:
        iGamma::sRGB().applyOnImage(m_curve, false);
        break;
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
        dflWarning(tr("Unknown photo scale"));
        return Linear;
    }
}

Photo *Photo::findReference(QVector<Photo> &photos)
{
    int p = -1;
    int count = photos.count();
    if (0 == count)
        return NULL;
    for (int i = 0 ; i < count ; ++i) {
        QString tag = photos[i].getTag(TAG_TREAT);
        if (tag == TAG_TREAT_DISCARDED ||
            tag == TAG_TREAT_ERROR)
            continue;
        if (tag == TAG_TREAT_REFERENCE) {
            p = i;
            break;
        }
    }
    if ( -1 == p )
        for (int i = 0 ; i < count ; ++i) {
            QString tag = photos[i].getTag(TAG_TREAT);
            if (tag != TAG_TREAT_DISCARDED &&
                tag != TAG_TREAT_ERROR) {
                p = i;
                break;
            }
        }
    if ( -1 == p )
        return NULL;
    return &photos[p];
}

Photo *Photo::findReference(Photo **photos, int count)
{
    int p = -1;
    for (int i = 0 ; i < count ; ++i) {
        if (0 == photos[i])
            continue;
        if (photos[i]->getTag(TAG_TREAT) == TAG_TREAT_REFERENCE) {
            p = i;
            break;
        }
    }
    if ( -1 == p ) {
        for (int i = 0 ; i < count ; ++i ) {
            if (photos[i] &&
                photos[i]->getTag(TAG_TREAT) != TAG_TREAT_DISCARDED &&
                photos[i]->getTag(TAG_TREAT) != TAG_TREAT_ERROR) {
                p = i;
                break;
            }
        }
    }
    if ( -1 == p )
        return NULL;
    return photos[p];
}

void ResetImage(Magick::Image &image)
{
    QElapsedTimer timer;
      timer.start();
    int w = image.columns();
    int h = image.rows();
    /*
     * modifyImage() seems to be armfull on windows (see 181e16ffd7d2053d318f807a3e9c0d40af35ac63)
     * but seems to be quicker than creating a new image, at least on OS X
     */
#if defined(DF_WINDOWS)
    image = Magick::Image(Magick::Geometry(w, h), Magick::Color(0,0,0));
    image.quantizeColorSpace(Magick::RGBColorspace);
#else
    image.modifyImage();
#endif
    dflDebug("Reset image(%d, %d) cost: %lld ms", w, h, timer.elapsed());
}

bool OnDiskCache()
{
    return false;
}

bool OnDiskCache(const Magick::Image &image)
{
    bool onDisk = MagickCore::GetImagePixelCacheType(const_cast<Magick::Image&>(image).image()) == MagickCore::DiskCache;
    if (onDisk)
        dflDebug(Photo::tr("Image cache is on disk, threading disabled"));
    return onDisk;
}

bool OnDiskCache(const Magick::Image &image1, const Magick::Image &image2)
{
    return OnDiskCache(image1) || OnDiskCache(image2);
}

bool OnDiskCache(const Magick::Image &image1, const Magick::Image &image2, const Magick::Image &image3)
{
    return OnDiskCache(image1) || OnDiskCache(image2) || OnDiskCache(image3);
}

bool OnDiskCache(const Magick::Image &image1, const Magick::Image &image2, const Magick::Image &image3, const Magick::Image &image4)
{
    return OnDiskCache(image1) || OnDiskCache(image2) || OnDiskCache(image3) || OnDiskCache(image4);
}

bool OnDiskCache(const Magick::Image &image1, const Magick::Image &image2, const Magick::Image &image3, const Magick::Image &image4, const Magick::Image &image5)
{
    return OnDiskCache(image1) || OnDiskCache(image2) || OnDiskCache(image3) || OnDiskCache(image4) || OnDiskCache(image5);
}
bool OnDiskCache(const Magick::Image &image1, const Magick::Image &image2, const Magick::Image &image3, const Magick::Image &image4, const Magick::Image &image5, const Magick::Image &image6)
{
    return OnDiskCache(image1) || OnDiskCache(image2) || OnDiskCache(image3) || OnDiskCache(image4) || OnDiskCache(image5) || OnDiskCache(image6);
}

int DfThreadLimit()
{
    return preferences->getNumThreads();
}
