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
#include "workerintegration.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "algorithm.h"
#include "hdr.h"
#include "transformview.h"
#include "cielab.h"
#include <Magick++.h>
#include <cmath>

#include <QVector>
#include <QPointF>
#include <QRectF>

using Magick::Quantum;

WorkerIntegration::WorkerIntegration(OpIntegration::RejectionType rejectionType,
                                     qreal upper,
                                     qreal lower,
                                     OpIntegration::NormalizationType normalizationType,
                                     qreal customNormalizationValue,
                                     bool outputHDR,
                                     qreal scale,
                                     QThread *thread,
                                     OpIntegration *op) :
    OperatorWorker(thread, op),
    m_rejectionType(rejectionType),
    m_upper(upper),
    m_lower(lower),
    m_normalizationType(normalizationType),
    m_customNormalizationValue(customNormalizationValue),
    m_outputHDR(outputHDR),
    m_integrationPlane(0),
    m_countPlane(0),
    m_w(0),
    m_h(0),
    m_offX(0),
    m_offY(0),
    m_scale(scale)
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

//Debug only, it breaks process with spurious points
//#define TRANSFORM_POINTS

bool WorkerIntegration::play_onInput(int idx)
{
    Q_UNUSED(idx);
    Q_ASSERT( idx == 0 );
    int photoCount = 0;
    int photoN = 0;
    Q_ASSERT( m_inputs.count() == 1 );
    photoCount=m_inputs[0].count();

    QVector<QPointF> reference;
#ifdef TRANSFORM_POINTS
    QVector<QPointF> transformed;
#endif
    Photo *refPhoto = Photo::findReference(m_inputs[0]);
    if (refPhoto) {
        reference = refPhoto->getPoints();
    }
    else {
        //no photo to process. not an error
        emitSuccess();
        return false;
    }
    foreach(Photo photo, m_inputs[0]) {
        if ( aborted() ) {
            emitFailure();
            return false;
        }
        if ( photo.getScale() == Photo::NonLinear ) {
            dflWarning(tr("%0 is non-linear").arg(photo.getIdentity()));
        }
        QVector<QPointF> points = photo.getPoints();

        try {
            if ( ! m_integrationPlane ) {
                createPlanes(refPhoto->image());
            }
            dfl_block int line = 0;

            bool hdr = photo.getScale() == Photo::HDR;
            bool hdrExposureAltered = false;
            QString hdrCompStr = photo.getTag(TAG_HDR_COMP);
            QString hdrHighStr = photo.getTag(TAG_HDR_HIGH);
            QString hdrLowStr = photo.getTag(TAG_HDR_LOW);
            qreal hdrComp = 1,
                    hdrHigh = QuantumRange,
                    hdrLow = 0;
            if ( ! hdrCompStr.isEmpty() && !hdrHighStr.isEmpty() && !hdrLowStr.isEmpty() ) {
                hdrExposureAltered = true;
                hdrComp = hdrCompStr.toDouble();
                hdrHigh = hdrHighStr.toDouble() * QuantumRange;
                hdrLow = hdrLowStr.toDouble() * QuantumRange;
            }
            std::shared_ptr<TransformView> view(new TransformView(photo, m_scale, reference));
            if (view->inError()) {
                dflError(tr("view in error"));
                continue;
            }
            if (!view->loadPixels()) {
                dflError(tr("unable to load pixels"));
                continue;
            }
#ifdef TRANSFORM_POINTS
            {
                qreal x, y;
                view->map(0,0, &x, &y);
                dflInfo("=> corner 1 in destination: %f, %f",x ,y);
                view->map(m_w,0, &x, &y);
                dflInfo("=> corner 2 in destination: %f, %f",x ,y);
                view->map(m_w,m_h, &x, &y);
                dflInfo("=> corner 3 in destination: %f, %f",x ,y);
                view->map(0,m_h, &x, &y);
                dflInfo("=> corner 4 in destination: %f, %f",x ,y);
                for (int i = 0, s = points.count() ; i < s ; ++i) {
                    qreal x, y;
                    view->invMap(points[i].x(), points[i].y(), &x, &y);
                    transformed.push_back(QPointF(x, y));
                }
            }
#endif

#define SUBPXL(plane, x,y,c) plane[(y)*m_w*3+(x)*3+(c)]
            dfl_parallel_for(y, 0, m_h, 4, (), {
                for ( int x = 0 ; x < m_w ; ++x ) {
                    bool defined;
                    Magick::PixelPacket pixel = view->getPixel(x,y,&defined);
                    if (!defined)
                        continue;
                    integration_plane_t red, green, blue;
                    if ( hdr ) {
                     red = fromHDR(pixel.red);
                     green = fromHDR(pixel.green);
                     blue = fromHDR(pixel.blue);
                    }
                    else {
                        red = pixel.red;
                        green = pixel.green;
                        blue = pixel.blue;
                    }
                    if ( hdrExposureAltered ) {
                        qreal lum = LUMINANCE(red, green, blue);
                        if ( lum >= hdrLow && lum <= hdrHigh ) {
                            SUBPXL(m_integrationPlane,x,y,0) += red / hdrComp;
                            ++SUBPXL(m_countPlane,x,y,0);
                            SUBPXL(m_integrationPlane,x,y,1) += green / hdrComp;
                            ++SUBPXL(m_countPlane,x,y,1);
                            SUBPXL(m_integrationPlane,x,y,2) += blue / hdrComp;
                            ++SUBPXL(m_countPlane,x,y,2);
                        }
                    }
                    else {
                        SUBPXL(m_integrationPlane,x,y,0) += red;
                        SUBPXL(m_integrationPlane,x,y,1) += green;
                        SUBPXL(m_integrationPlane,x,y,2) += blue;
                        ++SUBPXL(m_countPlane,x,y,0);
                        ++SUBPXL(m_countPlane,x,y,1);
                        ++SUBPXL(m_countPlane,x,y,2);
                    }
                }
                dfl_critical_section(
                {
                    ++line;
                    if ( 0 == line % 100 )
                        emitProgress(photoN, photoCount, line, m_h);
                });
            });
            ++photoN;
        }
        catch (std::exception &e) {
            setError(photo, e.what());
            emitFailure();
            return false;
        }
    }
    try {
        Photo newPhoto(Photo::Linear);
        newPhoto.setIdentity(m_operator->uuid());
        newPhoto.createImage(m_w, m_h);
        newPhoto.setTag(TAG_NAME, tr("Integration"));
        Magick::Image& newImage = newPhoto.image();
        std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(newImage));
        qreal mul = ( m_normalizationType == OpIntegration::Custom ? m_customNormalizationValue : 1. );
        dfl_parallel_for(y, 0, m_h, 4, (newImage), {
            Magick::PixelPacket *pixels = pixel_cache->get(0, y, m_w, 1);
            for ( int x = 0 ; x < m_w ; ++x ) {
                Q_ASSERT( y*m_w*3+x*3+2 < m_w*m_h*3);
                if (m_outputHDR) {
                    pixels[x].red = ( m_countPlane[y*m_w*3+x*3+0] )
                        ? toHDR(mul*m_integrationPlane[y*m_w*3+x*3+0]/m_countPlane[y*m_w*3+x*3+0])
                        : 0;
                    pixels[x].green = ( m_countPlane[y*m_w*3+x*3+1] )
                        ? toHDR(mul*m_integrationPlane[y*m_w*3+x*3+1]/m_countPlane[y*m_w*3+x*3+1])
                        : 0;
                    pixels[x].blue = ( m_countPlane[y*m_w*3+x*3+2] )
                        ? toHDR(mul*m_integrationPlane[y*m_w*3+x*3+2]/m_countPlane[y*m_w*3+x*3+2])
                        : 0;
                }
                else {
                    pixels[x].red = ( m_countPlane[y*m_w*3+x*3+0] )
                        ? clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+0]/m_countPlane[y*m_w*3+x*3+0], 0, QuantumRange)
                        : 0;
                    pixels[x].green = ( m_countPlane[y*m_w*3+x*3+1] )
                        ? clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+1]/m_countPlane[y*m_w*3+x*3+1], 0, QuantumRange)
                        : 0;
                    pixels[x].blue = ( m_countPlane[y*m_w*3+x*3+2] )
                        ? clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+2]/m_countPlane[y*m_w*3+x*3+2], 0, QuantumRange)
                        : 0;
                }
            }
            pixel_cache->sync();
        });
        if (m_outputHDR)
            newPhoto.setScale(Photo::HDR);
#ifdef TRANSFORM_POINTS
    newPhoto.setPoints(transformed);
#endif
        outputPush(0, newPhoto);
    }
    catch (std::exception &e) {
        dflError("%s", e.what());
        emitFailure();
        return false;
    }

    emitSuccess();
    return true;
}

void WorkerIntegration::createPlanes(Magick::Image &image)
{
    m_w = image.columns() * m_scale;
    m_h = image.rows() * m_scale;
    m_integrationPlane = new integration_plane_t[m_w*m_h*3];
    m_countPlane = new int[m_w*m_h*3];
    ::memset(m_integrationPlane, 0, m_w*m_h*3*sizeof(integration_plane_t));
    ::memset(m_countPlane, 0, m_w*m_h*3*sizeof(int));
    dflDebug(tr("Plane dim: w:%0, h:%1, sz:%2").arg(m_w).arg(m_h).arg(m_w*m_h*3));
    for ( int i = 0 ; i < m_w*m_h ; ++i ) {
        m_integrationPlane[i] = m_countPlane[i] = 0;
    }
}
