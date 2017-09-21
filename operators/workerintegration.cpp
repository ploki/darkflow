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
    m_minPlane(0),
    m_maxPlane(0),
    m_averagePlane(0),
    m_stdDevPlane(0),
    m_w(0),
    m_h(0),
    m_offX(0),
    m_offY(0),
    m_scale(scale)
{
    dflWarning(tr("H: %0, L: %1").arg(m_upper).arg(m_lower));
}

WorkerIntegration::~WorkerIntegration()
{
    delete[] m_integrationPlane;
    delete[] m_countPlane;
    delete[] m_minPlane;
    delete[] m_maxPlane;
    delete[] m_averagePlane;
    delete[] m_stdDevPlane;
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
    int photoN;
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
    enum Phase {
        PhaseMinMax = 0,
        PhaseMean,
        PhaseStdDev,
        PhaseIntegration,
        LastPhase
    };
    bool skip[LastPhase] = {};
    int nPhases = LastPhase;
    switch (m_rejectionType) {
    case OpIntegration::AverageDeviation:
        skip[PhaseStdDev] = true;
        --nPhases;
        // Falls through
    case OpIntegration::SigmaClipping:
        skip[PhaseMinMax] = true;
        --nPhases;
        break;
    default:
        dflError(tr("Unknown rejection algorithm"));
        // Falls through
    case OpIntegration::NoRejection:
        skip[PhaseMinMax] = true;
        --nPhases;
        // Falls through
    case OpIntegration::MinMax:
        skip[PhaseMean] = true;
        --nPhases;
        skip[PhaseStdDev] = true;
        --nPhases;
        break;
    }
    int phaseN=0;
    dfl_block long totalPixels=0;
    dfl_block long rejected=0;
    for (int phase = PhaseMinMax ; phase < LastPhase ; ++phase) {
        photoN = 0;
        if (skip[phase])
            continue;
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
                bool hdrAutomatic = false;
                QString hdrCompStr = photo.getTag(TAG_HDR_COMP);
                QString hdrHighStr = photo.getTag(TAG_HDR_HIGH);
                QString hdrLowStr = photo.getTag(TAG_HDR_LOW);
                QString hdrAutomaticStr = photo.getTag(TAG_HDR_AUTO);
                qreal hdrComp = 1,
                        hdrHigh = QuantumRange,
                        hdrLow = 0;
                if ( !hdrCompStr.isEmpty() &&
                     !hdrHighStr.isEmpty() &&
                     !hdrLowStr.isEmpty() &&
                     !hdrAutomaticStr.isEmpty()) {
                    hdrExposureAltered = true;
                    hdrComp = hdrCompStr.toDouble();
                    hdrHigh = hdrHighStr.toDouble() * QuantumRange;
                    hdrLow = hdrLowStr.toDouble() * QuantumRange;
                    hdrAutomatic = !!hdrAutomaticStr.toInt();
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

                Photo *rejPhoto = NULL;
                Ordinary::Pixels *rejCache = NULL;
                Magick::PixelPacket *rejPixels = NULL;
                if ( m_rejectionType != OpIntegration::NoRejection &&
                     phase == PhaseIntegration ) {
                    rejPhoto = new Photo(photo);
                    ResetImage(rejPhoto->image());
                    rejCache = new Ordinary::Pixels(rejPhoto->image());
                    rejPixels = rejCache->get(0, 0, m_w, m_h);
                }
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
                            if ( hdrAutomatic || (lum >= hdrLow && lum <= hdrHigh) ) {
                                red/=hdrComp;
                                green/=hdrComp;
                                blue/=hdrComp;
                            }
                            else {
                                 continue;
                            }
                         }
                         switch (phase) {
                             case PhaseIntegration: {
                                 double rgb[3] = { red, green, blue };
                                 for (int i = 0 ; i < 3 ; ++i) {
                                     bool reject = true;
                                     switch(m_rejectionType) {
                                         default:
                                         case OpIntegration::NoRejection:
                                         reject  = false;
                                         break;
                                         case OpIntegration::MinMax:
                                         if ( rgb[i] > SUBPXL(m_minPlane,x,y,i) &&
                                              rgb[i] < SUBPXL(m_maxPlane,x,y,i) )
                                             reject = false;
                                         break;
                                         case OpIntegration::AverageDeviation:
                                         if ( rgb[i] >= SUBPXL(m_averagePlane,x,y,i)/m_lower &&
                                              rgb[i] <= SUBPXL(m_averagePlane,x,y,i)*m_upper)
                                             reject = false;
                                         break;
                                         case OpIntegration::SigmaClipping:
                                         if ( rgb[i] >= SUBPXL(m_averagePlane,x,y,i)-SUBPXL(m_stdDevPlane,x,y,i)*m_lower &&
                                              rgb[i] <= SUBPXL(m_averagePlane,x,y,i)+SUBPXL(m_stdDevPlane,x,y,i)*m_upper)
                                             reject = false;
                                         break;
                                     }
                                     atomic_incr(&totalPixels);
                                     if (!reject) {
                                         SUBPXL(m_integrationPlane,x,y,i) += rgb[i];
                                         ++SUBPXL(m_countPlane,x,y,i);
                                         if (rejPixels) {
                                             switch(i) {
                                                 case 0:
                                                 rejPixels[y*m_w+x].red = 0; break;
                                                 case 1:
                                                 rejPixels[y*m_w+x].green = 0; break;
                                                 case 2:
                                                 rejPixels[y*m_w+x].blue = 0; break;
                                             }
                                         }
                                     }
                                     else {
                                        atomic_incr(&rejected);
                                        if (rejPixels) {
                                            switch(i) {
                                                case 0:
                                                rejPixels[y*m_w+x].red = pixel.red; break;
                                                case 1:
                                                rejPixels[y*m_w+x].green = pixel.green; break;
                                                case 2:
                                                rejPixels[y*m_w+x].blue = pixel.blue; break;
                                            }
                                        }
                                     }
                                 }
                                 break;
                             }
                             case PhaseMinMax:
                             SUBPXL(m_minPlane,x,y,0) = qMin(SUBPXL(m_minPlane,x,y,0), red);
                             SUBPXL(m_minPlane,x,y,1) = qMin(SUBPXL(m_minPlane,x,y,1), green);
                             SUBPXL(m_minPlane,x,y,2) = qMin(SUBPXL(m_minPlane,x,y,2), blue);
                             SUBPXL(m_maxPlane,x,y,0) = qMax(SUBPXL(m_maxPlane,x,y,0), red);
                             SUBPXL(m_maxPlane,x,y,1) = qMax(SUBPXL(m_maxPlane,x,y,1), green);
                             SUBPXL(m_maxPlane,x,y,2) = qMax(SUBPXL(m_maxPlane,x,y,2), blue);
                             break;
                             case PhaseMean:
                             SUBPXL(m_averagePlane,x,y,0) += red;
                             SUBPXL(m_averagePlane,x,y,1) += green;
                             SUBPXL(m_averagePlane,x,y,2) += blue;
                             ++SUBPXL(m_countPlane,x,y,0);
                             ++SUBPXL(m_countPlane,x,y,1);
                             ++SUBPXL(m_countPlane,x,y,2);
                             break;
                             case PhaseStdDev:
                             SUBPXL(m_stdDevPlane,x,y,0) += pow(red-SUBPXL(m_averagePlane,x,y,0), 2);
                             SUBPXL(m_stdDevPlane,x,y,1) += pow(green-SUBPXL(m_averagePlane,x,y,1), 2);
                             SUBPXL(m_stdDevPlane,x,y,2) += pow(blue-SUBPXL(m_averagePlane,x,y,2), 2);
                             ++SUBPXL(m_countPlane,x,y,0);
                             ++SUBPXL(m_countPlane,x,y,1);
                             ++SUBPXL(m_countPlane,x,y,2);
                             break;
                         }
                     }
                    dfl_critical_section(
                    {
                        ++line;
                        if ( 0 == line % 100 )
                            emitProgress(phaseN*photoCount+photoN, photoCount*nPhases, line, m_h);
                    });
                });
                if (rejPhoto) {
                    rejCache->sync();
                    outputPush(1, *rejPhoto);
                    delete rejCache;
                    delete rejPhoto;
                }
                ++photoN;
            }
            catch (std::exception &e) {
                setError(photo, e.what());
                emitFailure();
                return false;
            }
        }
        if (phase == PhaseMean) {
            for(int i=0, s=m_w*m_h*3 ; i < s ; ++i) {
                if (m_countPlane[i])
                    m_averagePlane[i] /= m_countPlane[i];
                else
                    m_averagePlane[i] = 0;
                m_countPlane[i] = 0;
            }
        }
        else if (phase == PhaseStdDev) {
            for(int i=0, s=m_w*m_h*3 ; i < s ; ++i) {
                if (m_countPlane[i])
                    m_stdDevPlane[i] = sqrt(m_stdDevPlane[i]/m_countPlane[i]);
                else
                    m_stdDevPlane[i] = 0;
                m_countPlane[i] = 0;
            }
        }
        ++phaseN;
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
    dflInfo(tr("Integrated %0 pixels. rejected: %1 (%2%)")
            .arg(totalPixels)
            .arg(rejected)
            .arg(100.*rejected/totalPixels));
    emitSuccess();
    return true;
}

void WorkerIntegration::createPlanes(Magick::Image &image)
{
    m_w = image.columns() * m_scale;
    m_h = image.rows() * m_scale;
    m_integrationPlane = new integration_plane_t[m_w*m_h*3]();
    m_countPlane = new int[m_w*m_h*3]();
    switch(m_rejectionType) {
    case OpIntegration::MinMax:
        m_minPlane = new integration_plane_t[m_w*m_h*3]();
        m_maxPlane = new integration_plane_t[m_w*m_h*3]();
        for (int i = 0, s = m_w*m_h*3 ; i < s ; ++i) {
            m_minPlane[i] = QuantumRange;
            m_maxPlane[i] = 0;
        }
        break;
    case OpIntegration::SigmaClipping:
        m_stdDevPlane = new integration_plane_t[m_w*m_h*3]();
        // Falls through
    case OpIntegration::AverageDeviation:
        m_averagePlane = new integration_plane_t[m_w*m_h*3]();
    default:break;
    }
    for (int i = 0, s = m_w*m_h*3 ; i < s ; ++i) {
        if (m_integrationPlane) m_integrationPlane[i] = 0;
        if (m_countPlane) m_countPlane[i] = 0;
        if (m_minPlane) m_minPlane[i] = 0;
        if (m_maxPlane) m_maxPlane[i] = 0;
        if (m_stdDevPlane) m_stdDevPlane[i] = 0;
        if (m_averagePlane) m_averagePlane[i] = 0;
    }
    dflDebug(tr("Plane dim: w:%0, h:%1, sz:%2").arg(m_w).arg(m_h).arg(m_w*m_h*3));
}
