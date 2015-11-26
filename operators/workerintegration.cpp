#include "workerintegration.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "algorithm.h"
#include <Magick++.h>

#include <QVector>
#include <QPointF>
#include <QRectF>

using Magick::Quantum;

WorkerIntegration::WorkerIntegration(OpIntegration::RejectionType rejectionType,
                                     qreal upper,
                                     qreal lower,
                                     OpIntegration::NormalizationType normalizationType,
                                     qreal customNormalizationValue,
                                     QThread *thread,
                                     OpIntegration *op) :
    OperatorWorker(thread, op),
    m_rejectionType(rejectionType),
    m_upper(upper),
    m_lower(lower),
    m_normalizationType(normalizationType),
    m_customNormalizationValue(customNormalizationValue),
    m_integrationPlane(0),
    m_countPlane(0),
    m_w(0),
    m_h(0),
    m_offX(0),
    m_offY(0)
{
}

WorkerIntegration::~WorkerIntegration()
{
    delete[] m_integrationPlane;
    delete[] m_countPlane;
}

QRectF WorkerIntegration::computePlanesDimensions()
{
    bool offSet = false;
    qreal x1=0,y1=0,x2=0,y2=0;
    foreach(Photo photo, m_inputs[0]) {
        Magick::Image& image = photo.image();
        int w = image.columns();
        int h = image.rows();
        qreal x = 0, y = 0;
        QVector<QPointF> points = photo.getPoints();
        if ( points.count() == 1 ) {
            if ( !offSet ) {
                m_offX=points[0].x();
                m_offY=points[0].y();
                offSet = true;
            }
            x = points[0].x() - m_offX;
            y = points[0].y() - m_offY;
        }
        if ( x < x1 )
            x1 = x;
        if ( y < y1 )
            y1 = y;
        if ( x+w > x2 )
            x2 = x+w;
        if ( y+h > y2 )
            y2 = y+h;
    }
    return QRectF(x1,y1,x2-x1,y2-y1);
}

bool WorkerIntegration::play_onInput(int idx)
{
    Q_UNUSED(idx);
    Q_ASSERT( idx == 0 );
    int photoCount = 0;
    int photoN = 0;
    Q_ASSERT( m_inputs.count() == 1 );
    photoCount=m_inputs[0].count();

    bool firstFrame=true;
    qreal ff_x = 0, ff_y = 0;

    foreach(Photo photo, m_inputs[0]) {
        if ( aborted() ) {
            emitFailure();
            return false;
        }

        QVector<QPointF> points = photo.getPoints();
        qreal lcx=0, lcy=0;
        if ( points.count() > 0 ) {
            lcx = points[0].x();
            lcy = points[0].y();
            if ( firstFrame ) {
                firstFrame = false;
                ff_x = lcx;
                ff_y = lcy;
            }
        }
        int cx = round(lcx - ff_x);
        int cy = round(lcy - ff_y);

        Magick::Image& image = photo.image();
        if ( ! m_integrationPlane ) {
            createPlanes(image);
        }
        Magick::Pixels pixel_cache(image);
        int line = 0;

#define SUBPXL(plane, x,y,c) plane[(y)*m_w*3+(x)*3+(c)]
#pragma omp parallel for
        for ( int y = 0 ; y < m_h ; ++y ) {
            if ( y+cy < 0 || y+cy >= m_h ) continue;
            Magick::PixelPacket *pixels = pixel_cache.get(0, y+cy, m_w, 1);
            if ( !pixels ) continue;
            for ( int x = 0 ; x < m_w ; ++x ) {
                if ( x+cx < 0 || x+cx >= m_w ) continue;
                SUBPXL(m_integrationPlane,x,y,0) += pixels[x+cx].red;
                SUBPXL(m_integrationPlane,x,y,1) += pixels[x+cx].green;
                SUBPXL(m_integrationPlane,x,y,2) += pixels[x+cx].blue;
                ++SUBPXL(m_countPlane,x,y,0);
                ++SUBPXL(m_countPlane,x,y,1);
                ++SUBPXL(m_countPlane,x,y,2);
            }
#pragma omp critical
            {
                ++line;
                if ( 0 == line % 100 )
                    emitProgress(photoN, photoCount, line, m_h);
            }
        }
#pragma omp barrier
        ++photoN;
    }
    Photo newPhoto;
    newPhoto.setIdentity(m_operator->uuid());
    newPhoto.createImage(m_w, m_h);
    Magick::Image& newImage = newPhoto.image();
    newImage.modifyImage();
    Magick::Pixels pixel_cache(newImage);
    qreal mul = ( m_normalizationType == OpIntegration::Custom ? m_customNormalizationValue : 1. );
#pragma omp parallel for
    for ( int y = 0 ; y < m_h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0, y, m_w, 1);
        for ( int x = 0 ; x < m_w ; ++x ) {
            Q_ASSERT( y*m_w*3+x*3+2 < m_w*m_h*3);
            pixels[x].red   =
                    clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+0]/m_countPlane[y*m_w*3+x*3+0], 0, QuantumRange);
            pixels[x].green =
                    clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+1]/m_countPlane[y*m_w*3+x*3+1], 0, QuantumRange);
            pixels[x].blue  =
                    clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+2]/m_countPlane[y*m_w*3+x*3+2], 0, QuantumRange);
        }
    }
#pragma omp barrier
    newPhoto.setTag("Name", "Integration");
    m_outputs[0].push_back(newPhoto);
    emitSuccess();
    return true;
}

void WorkerIntegration::createPlanes(Magick::Image &image)
{
    m_w = image.columns();
    m_h = image.rows();
    m_integrationPlane = new quantum_t[m_w*m_h*3];
    m_countPlane = new int[m_w*m_h*3];
    ::memset(m_integrationPlane, 0, m_w*m_h*3*sizeof(quantum_t));
    ::memset(m_countPlane, 0, m_w*m_h*3*sizeof(int));
    qDebug(QString("Plane dim: w:%0, h:%1, sz:%2").arg(m_w).arg(m_h).arg(m_w*m_h*3).toLatin1());
    for ( int i = 0 ; i < m_w*m_h ; ++i ) {
        m_integrationPlane[i] = m_countPlane[i] = 0;
    }
}
