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
#include "ports.h"
#ifdef DF_WINDOWS
# include <QProcess>
# define PROCESSCLASS QProcess
#else
# include "posixspawn.h"
# define PROCESSCLASS PosixSpawn
#endif
#include <QRegularExpression>

#include "rawinfo.h"



RawInfo::RawInfo(QObject *parent) :
    QObject(parent),
    m_isoSpeed(0),
    m_shutterSpeed(0),
    m_aperture(0),
    m_focal(0),
    m_daylightMultipliers(),
    m_cameraMultipliers(),
    m_filterPattern("")
{
}

qreal RawInfo::isoSpeed() const
{
    return m_isoSpeed;
}

qreal RawInfo::shutterSpeed() const
{
    return m_shutterSpeed;
}

qreal RawInfo::aperture() const
{
    return m_aperture;
}

qreal RawInfo::focal() const
{
    return m_focal;
}

RawInfo::Multipliers RawInfo::daylightMultipliers() const
{
    return m_daylightMultipliers;
}

RawInfo::Multipliers RawInfo::cameraMultipliers() const
{
    return m_cameraMultipliers;
}

QString RawInfo::camera() const
{
    return m_camera;
}

QString RawInfo::timestamp() const
{
    return m_timestamp;
}

QString RawInfo::filterPattern() const
{
    return m_filterPattern;
}

bool RawInfo::probeFile(const QString &filename)
{
    /*
     * Info à lire:
    Camera: NIKON D700
    Timestamp: Wed Jun 18 22:54:47 2008
    ISO speed: 12800
    Shutter: 1/200.0 sec
    Aperture: f/10.0
    Focal length: 200.0 mm
    Daylight multipliers: 2.064871 0.932310 1.112389
    Camera multipliers: 1.503906 1.000000 1.648438 0.000000
    */
    QString dcraw_executable("dcraw");
    QStringList arguments;
    arguments << "-i" << "-v" << filename;
    PROCESSCLASS dcraw;
    dcraw.start(dcraw_executable, arguments, PROCESSCLASS::ReadOnly|PROCESSCLASS::Text);
    if ( !dcraw.waitForStarted() )
        return false;
    dcraw.waitForFinished();
    char buf[1024];
    qint64 len;
    while ( (len = dcraw.readLine(buf, sizeof(buf)) ) > 0 ) {
        if ( buf[len-1] == '\n')
            buf[len-1] = '\0';
        QString str(buf);
        QStringList elem = str.split(" ");

        if ( str.contains(QRegularExpression("^ISO speed: ") ) ) {
            m_isoSpeed = elem[2].toDouble();
        }
        else if ( str.contains(QRegularExpression("^Shutter: ") ) ) {
            QStringList sh = elem[1].split("/");
            m_shutterSpeed = sh[0].toDouble();
            if ( sh.count() == 2 ) m_shutterSpeed/=sh[1].toDouble();
        }
        else if ( str.contains(QRegularExpression("^Aperture: ") ) ) {
            QStringList sh = elem[1].split("/");
            m_aperture = sh[1].toDouble();
        }
        else if ( str.contains(QRegularExpression("^Focal length: ") ) ) {
            m_focal = elem[2].toDouble();
        }
        else if ( str.contains(QRegularExpression("^Daylight multipliers: ") ) ) {
            m_daylightMultipliers.r = elem[2].toDouble();
            m_daylightMultipliers.g = elem[3].toDouble();
            m_daylightMultipliers.b = elem[4].toDouble();
            if ( elem.count() == 6 ) {
                qreal div = elem[5].toDouble();
                if ( div != 0.L ) {
                    m_daylightMultipliers.r/=div;
                    m_daylightMultipliers.g/=div;
                    m_daylightMultipliers.b/=div;
                }
            }
        }
        else if ( str.contains(QRegularExpression("^Camera multipliers: ") ) ) {
            m_cameraMultipliers.r = elem[2].toDouble();
            m_cameraMultipliers.g = elem[3].toDouble();
            m_cameraMultipliers.b = elem[4].toDouble();
            if ( elem.count() == 6 ) {
                qreal div = elem[5].toDouble();
                if ( div != 0.L ) {
                    m_cameraMultipliers.r/=div;
                    m_cameraMultipliers.g/=div;
                    m_cameraMultipliers.b/=div;
                }
            }
        }
        else if ( str.contains(QRegularExpression("^Camera: ") ) ) {
            m_camera=buf+8;
        }
        else if ( str.contains(QRegularExpression("^TimeStamp: ") ) ) {
            m_timestamp=buf+11;
        }
        else if ( str.contains(QRegularExpression("^Filter pattern: ") ) ) {
            m_filterPattern=buf+16;
        }
    }
    return true;
}

RawInfo::Multipliers::Multipliers() : r(1.), g(1.), b(1.)
{
}
