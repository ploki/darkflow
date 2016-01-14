#include <QVector>
#include <QPointF>
#include "workergradientevaluation.h"
#include "algorithm.h"
#include "hdr.h"

WorkerGradientEvaluation::WorkerGradientEvaluation(qreal radius,
                                                   qreal altitude,
                                                   qreal pow_,
                                                   QThread *thread,
                                                   Operator *op) :
    OperatorWorker(thread, op),
    m_radius(radius),
    m_altitude(altitude),
    m_pow(pow_)
{
}


Photo WorkerGradientEvaluation::process(const Photo &srcPhoto, int p, int c)
{
    Photo inPhoto(srcPhoto);
    Photo outPhoto(srcPhoto);
    Magick::Image& in = inPhoto.image();
    Magick::Image& out = outPhoto.image();

    bool hdr = srcPhoto.getScale() == Photo::HDR;
    int w = in.columns();
    int h = in.rows();
    Magick::Pixels cache(in);
    const Magick::PixelPacket *pixels = cache.getConst(0, 0, w, h);

    QVector<QPointF> points = inPhoto.getPoints();
    int n_points =  points.count();
    QVector<Triplet<qreal> > colors(n_points);
    qreal radius_pow2 = m_radius*m_radius;

#pragma omp parallel for dfl_threads(4, in, out)
    for (int i = 0 ; i < n_points ; ++i ) {
        int count = 0;
        Triplet<qreal> color;
        Q_ASSERT(color.red == 0 && color.green == 0 && color.blue == 0);
        for (int y = points[i].y()-m_radius-1 ; y <= points[i].y()+m_radius+1 ; ++y ) {
            for (int x = points[i].x()-m_radius-1 ; x <= points[i].x()+m_radius+1 ; ++x ) {
                if ( x < 0 || x >= w || y < 0 || y >= h)
                    continue;
                qreal dx = points[i].x()-x;
                qreal dy = points[i].y()-y;
                if ( dx*dx+dy*dy >= radius_pow2 )
                    continue;
                if ( hdr ) {
                    color.red+=fromHDR(pixels[y*w+x].red);
                    color.green+=fromHDR(pixels[y*w+x].green);
                    color.blue+=fromHDR(pixels[y*w+x].blue);
                }
                else {
                    color.red+=(pixels[y*w+x].red);
                    color.green+=(pixels[y*w+x].green);
                    color.blue+=(pixels[y*w+x].blue);
                }
                ++count;
            }
        }
        if ( count ) {
            color.red/=count;
            color.green/=count;
            color.blue/=count;
        }
        else {
            dflWarning("missed a color");
        }
#pragma omp critical
        {
            colors[i] = color;
        }
    }

    ResetImage(out);
    Magick::Pixels out_cache(out);
    Magick::PixelPacket *pxl = out_cache.get(0, 0, w, h);

    int line=0;
    qreal altitude = m_altitude*m_altitude;
#pragma omp parallel for dfl_threads(4)
    for (int y = 0 ; y < h ; ++y ) {
        for ( int x = 0 ; x < w ; ++x ) {
            Triplet<qreal> color;
            qreal coef = 0;
            for ( int i = 0 ; i < n_points ; ++i ) {
                qreal px = points[i].x();
                qreal py = points[i].y();
                if ( x == px && y == py ) {
                    color = colors[i];
                    coef = 1;
                    break;
                }
                qreal dx = px-x;
                qreal dy = py-y;
                qreal dist = pow(dx*dx+dy*dy+altitude, m_pow);
                color.red +=  double(colors[i].red)/dist;
                color.green += double(colors[i].green)/dist;
                color.blue += double(colors[i].blue)/dist;
                coef+=1./dist;
            }
            if (hdr) {
                pxl[y*w+x].red = clamp<quantum_t>(toHDR(color.red/coef));
                pxl[y*w+x].green = clamp<quantum_t>(toHDR(color.green/coef));
                pxl[y*w+x].blue = clamp<quantum_t>(toHDR(color.blue/coef));
            }
            else {
                pxl[y*w+x].red = clamp<quantum_t>(color.red/coef);
                pxl[y*w+x].green = clamp<quantum_t>(color.green/coef);
                pxl[y*w+x].blue = clamp<quantum_t>(color.blue/coef);
            }

        }
#pragma omp critical
        {
            emitProgress(p, c, line++, h);
        }
    }
    out_cache.sync();

    outPhoto.setPoints(QVector<QPointF>());
    outPhoto.setTag(TAG_NAME, "Background of "+outPhoto.getTag(TAG_NAME));
    return outPhoto;
}
