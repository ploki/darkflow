#include "transformview.h"
#include "console.h"
#include "algorithm.h"
#include "hdr.h"

#include <QLineF>
#include <QGenericMatrix>

static inline
QGenericMatrix<3, 3, double>
invert(QGenericMatrix<3, 3, double> m, bool *invertedp)
{
    double det = m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) -
                 m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) +
                 m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));
   QGenericMatrix<3, 3, double> minv;
   if ( fabs(det) < 0.00001 ) {
       *invertedp = false;
   }
   else {
       *invertedp = true;
       double invdet = 1 / det;
       minv(0, 0) = (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) * invdet;
       minv(0, 1) = (m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2)) * invdet;
       minv(0, 2) = (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * invdet;
       minv(1, 0) = (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) * invdet;
       minv(1, 1) = (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0)) * invdet;
       minv(1, 2) = (m(1, 0) * m(0, 2) - m(0, 0) * m(1, 2)) * invdet;
       minv(2, 0) = (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * invdet;
       minv(2, 1) = (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) * invdet;
       minv(2, 2) = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * invdet;
   }
   return minv;
}

TransformView::TransformView(const Photo &photo, qreal scale, QVector<QPointF> ref, QObject *parent)
    : QObject(parent),
      m_photo(photo),
      m_transform(QTransform()),
      m_w(m_photo.image().columns()),
      m_h(m_photo.image().rows()),
      m_cache(0),
      m_pixels(0),
      m_error(false),
      m_hdr(photo.getScale() == Photo::HDR)
{
    QVector<QPointF> reference =ref;
    QVector<QPointF> current = m_photo.getPoints();

    int current_count = current.count();
    int reference_count = reference.count();
    if ( current_count != reference_count ) {
        if ( current_count == 0 ) {
            dflWarning(tr("Transformation points not defined for this image"));
        }
        else {
            m_error = true;
            dflError(tr("Transformation points count error"));
            return;
        }
    }
    else {
        switch(reference_count) {
        case 0:
            break;
        case 1:
            m_transform.translate(current[0].x()-reference[0].x(),
                    current[0].y()-reference[0].y());
            break;
        case 2: {
            QLineF r = QLineF(0, 0,
                              reference[1].x() - reference[0].x(),
                              reference[1].y() - reference[0].y());
            QLineF c = QLineF(0, 0,
                              current[1].x() - current[0].x(),
                              current[1].y() - current[0].y());

            qreal x = (reference[1].x() + reference[0].x())/2.,
                  y = (reference[1].y() + reference[0].y())/2.;
            qreal dx = (current[1].x()+current[0].x())/2.,
                  dy = (current[1].y()+current[0].y())/2.;
            qreal factor = c.length()/r.length();
            qreal angle = c.angleTo(r);

            m_transform.translate(dx, dy);
            m_transform.rotate(angle);
            m_transform.scale(factor, factor);
            m_transform.translate(-x, -y);
            break;
        }
        case 3: {
#if 0
            bool invertible;
            QTransform r(reference[0].x(), reference[1].x(), reference[2].x(),
                         reference[0].y(), reference[1].y(), reference[2].y(),
                         1, 1, 1);
            QTransform c(current[0].x(), current[1].x(), current[2].x(),
                         current[0].y(), current[1].y(), current[2].y(),
                         1, 1, 1);
            QTransform inverted = r.inverted(&invertible);
            if (invertible) {
                QTransform tmp = c * inverted;
                m_transform *= tmp.transposed();
            }
            else {
                dflError(tr("Points do not define a segment (singular matrix)"));
            }
#else
            double rc[] = {reference[0].x(), reference[1].x(), reference[2].x(),
                           reference[0].y(), reference[1].y(), reference[2].y(),
                           1, 1, 1};
            double cc[] = {current[0].x(), current[1].x(), current[2].x(),
                           current[0].y(), current[1].y(), current[2].y(),
                           1, 1, 1};
            bool invertible;
            QGenericMatrix<3, 3, double> r(rc);
            QGenericMatrix<3, 3, double> c(cc);
            QGenericMatrix<3, 3, double> inverted = invert(r, &invertible);
            if (invertible) {
                c = c * inverted;
                c = c.transposed();
                m_transform *= QTransform(c(0,0), c(0, 1), c(0,2),
                                          c(1,0), c(1, 1), c(1,2),
                                          c(2,0), c(2, 1), c(2,2));
            }
            else {
                dflError(tr("Points do not define a segment (singular matrix)"));
            }
#endif
/*
    //sanity check
            qreal tx, ty;
            m_transform.map(current[0].x(), current[0].y(), &tx, &ty);
            Q_ASSERT( fabs(reference[0].x() - tx) < 0.0001 );
            Q_ASSERT( fabs(reference[0].y() - ty) < 0.0001 );
            m_transform.map(current[1].x(), current[1].y(), &tx, &ty);
            Q_ASSERT( fabs(reference[1].x() - tx) < 0.0001 );
            Q_ASSERT( fabs(reference[1].y() - ty) < 0.0001 );
            */
            break;
        }
        default:
            dflError(tr("Too many points for transformation"));
        }
    }
    m_transform.scale(1/scale, 1/scale);

}

TransformView::~TransformView()
{
    delete m_cache;
}

QRectF TransformView::boundingBox()
{
    QRectF currentBoundingBox(0, 0, m_w, m_h);
    QTransform trans = m_transform.inverted();
    return trans.mapRect(currentBoundingBox);
}

bool TransformView::inError()
{
    if (!m_error) {
        dflInfo(tr("-------------------------------------"));
        dflInfo(tr("transformation matrix %0 %1 %2").arg(m_transform.m11()).arg(m_transform.m12()).arg(m_transform.m13()));
        dflInfo(tr("transformation matrix %0 %1 %2").arg(m_transform.m21()).arg(m_transform.m22()).arg(m_transform.m23()));
        dflInfo(tr("transformation matrix %0 %1 %2").arg(m_transform.m31()).arg(m_transform.m32()).arg(m_transform.m33()));
    }
    return m_error;
}

bool TransformView::loadPixels()
{
    m_cache = new Ordinary::Pixels(m_photo.image());
    if (m_cache)
        m_pixels = m_cache->getConst(0, 0, m_w, m_h);
    return m_pixels != 0;
}

void TransformView::map(qreal x, qreal y, qreal *tx, qreal *ty)
{
    m_transform.map(x,y,tx,ty);
}

void TransformView::invMap(qreal x, qreal y, qreal *tx, qreal *ty)
{
    QTransform trans = m_transform.inverted();
    trans.map(x,y,tx,ty);
}

Magick::PixelPacket TransformView::getPixel(int px, int py, bool *definedp)
{
    Magick::PixelPacket pixel;
    //g++-4.9 complains about {} initializer
    memset(&pixel, 0, sizeof(pixel));
    if ( m_transform.isIdentity() ) {
        if(definedp)
            *definedp=true;
        return m_pixels[py*m_w+px];
    }
    qreal sx, sy;
    qreal ex, ey;
    map(px,py, &sx, &sy);
    map(px+1,py+1, &ex, &ey);
    if ( sx < 0 || sx >= m_w ||
         sy < 0 || sy >= m_h ||
         ex < 0 || ex >= m_w ||
         ey < 0 || ey >= m_h) {
        if (definedp)
            *definedp = false;
        //qDebug("out of bound: p: %d,%d proj: %f,%f",px,py,sx,sy);
        return pixel;
    }

    if ( sx > ex ) qSwap(sx, ex);
    if ( sy > ey ) qSwap(sy, ey);
    if (definedp)
        *definedp = true;
    //qDebug("? y:(%f => %f), x:(%f => %f)\n> y:(%d => %d), x:(%d => %d)", sy,ey,sx,ex, int(sy),int(ey+1),int(sx),int(ex+1));
    qreal red = 0, green = 0, blue = 0;
    qreal s = 0;
    for ( int y = sy ; y <= ey ; ++y ) {
        qreal dy = (double(y+1) > ey ? ey : double(y+1))
                 - (double(y) < sy ? sy : double(y));
        for ( int x = sx ; x <= ex ; ++x ) {
            qreal dx = (double(x+1) > ex ? ex : double(x+1))
                     - (double(x) < sx ? sx : double(x));
            qreal ds = fabs(dx*dy);
            //qDebug("> y: %d, x: %d, dy: %f, dx: %f, ds: %f", y, x, dy, dx, ds);
            pixel = m_pixels[int(y)*m_w+int(x)];
            if (m_hdr) {
                red += ds*fromHDR(pixel.red);
                green += ds*fromHDR(pixel.green);
                blue += ds*fromHDR(pixel.blue);
            }
            else {
                red += ds*pixel.red;
                green += ds*pixel.green;
                blue += ds*pixel.blue;
            }
            s += ds;
        }
    }
    if (m_hdr) {
        pixel.red = toHDR(red/s);
        pixel.green = toHDR(green/s);
        pixel.blue = toHDR(blue/s);
    }
    else {
        pixel.red = clamp<quantum_t>(red/s);
        pixel.green = clamp<quantum_t>(green/s);
        pixel.blue = clamp<quantum_t>(blue/s);
    }
    return pixel;
}
