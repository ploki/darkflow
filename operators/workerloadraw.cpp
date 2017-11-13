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
#include <QStringList>
#include "ports.h"
#if defined(DF_WINDOWS) || defined(ANDROID)
# include <QProcess>
# define PROCESSCLASS QProcess
#else
# include "posixspawn.h"
# define PROCESSCLASS PosixSpawn
#endif
#include <QByteArray>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include "process.h"

#include <Magick++.h>

#include "workerloadraw.h"
#include "rawinfo.h"
#include "operatoroutput.h"
#include "oploadraw.h"
#include "photo.h"

WorkerLoadRaw::WorkerLoadRaw(QThread *thread, OpLoadRaw *op) :
    OperatorWorker(thread, op),
    m_loadraw(op)
{}

void WorkerLoadRaw::play()
{
    QVector<QString> collection = m_loadraw->getCollection().toVector();
    int s = collection.count();
    dfl_block int p = 0;
    dfl_block bool failure = false;

    QString badPixelsFileTemplate = preferences->tmpDir()
            + "/bad-pixels-map-XXXXXXXX.txt";
    QTemporaryFile badPixelsFile(badPixelsFileTemplate);
    if (!badPixelsFile.open()) {
        dflError(tr("Failed to create temporary file %0").arg(badPixelsFileTemplate));
        emitFailure();
        return;
    }
    foreach(Photo photo, m_inputs[0]) {
        Ordinary::Pixels src_cache(photo.image());
        int h = photo.image().columns();
        int w = photo.image().rows();
        for (int y = 0 ; y < h ; ++y) {
            for (int x = 0 ; x < w ; ++x) {
                const Magick::PixelPacket *pixels = src_cache.getConst(0, y, w, 1);
                if (!pixels) continue;
                if (pixels[x].red ||
                    pixels[x].green ||
                    pixels[x].blue) {
                    badPixelsFile.write(QString("%0 %1 0\n").arg(x).arg(y).toLocal8Bit());
                }
            }
        }
    }
    QString badPixelsFileName = badPixelsFile.fileName();
 dfl_parallel_for(i, 0, s, 1, (), {
        if ( failure || aborted() ) {
            failure = true;
            continue;
        }
        QByteArray data = convert(collection[i], badPixelsFileName);
        try {
            Magick::Blob blob(data.data(),data.length());
            if ( blob.data() == 0 || data.length() == 0 ) {
                failure = true;
                continue;
            }
            Photo::Gamma gamma;
            switch(m_loadraw->m_colorSpaceValue) {
            default:
            case OpLoadRaw::Linear: gamma = Photo::Linear; break;
            case OpLoadRaw::IUT_BT_709: gamma = Photo::IUT_BT_709; break;
            case OpLoadRaw::sRGB: gamma = Photo::sRGB; break;
            }
            Photo photo(blob, gamma);
            if ( !photo.isComplete() ) {
                failure = true;
                continue;
            }
            setTags(collection[i], photo);
            photo.setSequenceNumber(i);
            dfl_critical_section({
                emit progress(++p, s);
                outputPush(0, photo);
            });
        }
        catch (std::exception &e) {
            dflError("%s", e.what());
            failure = true;
        }
        catch (...) {
            dflError("Unknown exception");
            failure = true;
        }
    });

    if ( failure || aborted() ) {
        emitFailure();
    }
    else {
        outputSort(0);
        emitSuccess();
    }

}



QByteArray WorkerLoadRaw::convert(const QString &filename,
                                  const QString &badPixelsMapFile)
{
    QString dcraw_executable("dcraw");
    QStringList arguments;
    PROCESSCLASS dcraw;
    switch(m_loadraw->m_whiteBalanceValue ) {
    case OpLoadRaw::NoWhiteBalance:
        arguments << "-r" << "1" << "1" << "1" << "1";
        break;
    case OpLoadRaw::RawColors:
        arguments << "-o" << "0";
        break;
    case OpLoadRaw::Camera:
        arguments << "-w";
        break;
    case OpLoadRaw::Daylight:
        //it is the default
        break;
    }
    switch(m_loadraw->m_debayerValue) {
    case OpLoadRaw::NoDebayer:
        arguments << "-d";
        break;
    case OpLoadRaw::HalfSize:
        arguments << "-h";
        break;
    case OpLoadRaw::Low:
        arguments << "-q" << "0";
        break;
    case OpLoadRaw::VNG:
        arguments << "-q" << "1";
        break;
    case OpLoadRaw::PPG:
        arguments << "-q" << "2";
        break;
    case OpLoadRaw::AHD:
        arguments << "-q" << "3";
        break;
    }
    switch(m_loadraw->m_colorSpaceValue) {
    case OpLoadRaw::Linear:
        arguments << "-4";
        break;
    case OpLoadRaw::sRGB:
        arguments << "-g" << "2.4" << "12.92";
        // Falls through
    case OpLoadRaw::IUT_BT_709:
        arguments << "-6";
        break;
    }
    switch(m_loadraw->m_clippingValue) {
    case OpLoadRaw::ClipAuto:
        break;
    case OpLoadRaw::Clip16bit:
        arguments << "-S" << "65535";
        break;
    case OpLoadRaw::Clip15bit:
        arguments << "-S" << "32767";
        break;
    case OpLoadRaw::Clip14bit:
        arguments << "-S" << "16383";
        break;
    case OpLoadRaw::Clip13bit:
        arguments << "-S" << "8191";
        break;
    case OpLoadRaw::Clip12bit:
        arguments << "-S" << "4095";
        break;
    }

    /* darkness to 0 */
    arguments << "-k" << "0";
    /* orientation */
    arguments << "-t" << "0";
    /* write to standard output */
    arguments << "-c";
    /* write a TIFF with MD */
    //arguments << "-T";
    /* bad pixels map */
    arguments << "-P" << badPixelsMapFile;
    /* the RAW photo */
    arguments << filename;

    dcraw.start(dcraw_executable, arguments, PROCESSCLASS::ReadOnly);
    QByteArray data;
#ifndef DF_WINDOWS
    data = dcraw.readAllStandardOutput();
#endif
    dcraw.waitForFinished();
    int rc = dcraw.exitCode();
    if ( 0 != rc )
        dflError("dcraw failure: rc=%d", rc);
#ifdef DF_WINDOWS
    else
        data = dcraw.readAllStandardOutput();
#endif
    dflDebug("dcraw bytes read: %d", data.size());
    return data;
}

void WorkerLoadRaw::setTags(const QString &filename, Photo &photo)
{
    RawInfo info;
    QFileInfo finfo(filename);
    info.probeFile(filename);
    //photo.writeJPG("/tmp/"+finfo.fileName()+".jpg");
    photo.setIdentity(m_operator->uuid()+"/"+finfo.fileName());
    photo.setTag(TAG_NAME, finfo.fileName());
    photo.setTag(TAG_DIRECTORY, finfo.dir().path());
    photo.setTag(TAG_ISO_SPEED, QString("%0").arg(info.isoSpeed()));
    photo.setTag(TAG_SHUTTER, QString("%0").arg(info.shutterSpeed()));
    photo.setTag(TAG_APERTURE, QString("%0").arg(info.aperture()));
    photo.setTag(TAG_FOCAL_LENGTH, QString("%0").arg(info.focal()));
    photo.setTag(TAG_D65_R_MULTIPLIER, QString("%0").arg(info.daylightMultipliers().r));
    photo.setTag(TAG_D65_G_MULTIPLIER, QString("%0").arg(info.daylightMultipliers().g));
    photo.setTag(TAG_D65_B_MULTIPLIER, QString("%0").arg(info.daylightMultipliers().b));
    photo.setTag(TAG_CAMERA, info.camera());
    photo.setTag(TAG_TIMESTAMP, info.timestamp());
    photo.setTag(TAG_FILTER_PATTERN, info.filterPattern());
    photo.setTag(TAG_COLOR_SPACE, m_loadraw->getColorSpace());
    photo.setTag(TAG_SCALE, m_loadraw->m_colorSpaceValue == OpLoadRaw::Linear
                 ? TAG_SCALE_LINEAR
                 : TAG_SCALE_NONLINEAR);
    photo.setTag(TAG_DEBAYER, m_loadraw->getDebayer());
    photo.setTag(TAG_WHITE_BALANCE, m_loadraw->getWhiteBalance());
    if ( m_loadraw->m_debayerValue == OpLoadRaw::NoDebayer ) {
        photo.setTag(TAG_PIXELS, TAG_PIXELS_CFA);
    }
    else {
        photo.setTag(TAG_PIXELS, TAG_PIXELS_RGB);
    }
}
