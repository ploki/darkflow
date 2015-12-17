#include <QStringList>
#include <QProcess>
#include <QByteArray>
#include <QThread>
#include <QFileInfo>
#include <QDir>
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
    int p = 0;
    volatile bool failure = false;
#pragma omp parallel for shared(failure)
    for (int i = 0 ; i < s ; ++i) {
        if ( failure || aborted() ) {
            failure = true;
            continue;
        }
        QByteArray data = convert(collection[i]);
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
            photo.setIdentity(collection[i]);
            setTags(collection[i], photo);
            photo.setSequenceNumber(i);
#pragma omp critical
            {
                emit progress(++p, s);
                outputPush(0, photo);
            }
        }
        catch (std::exception &e) {
            dflError(e.what());
            failure = true;
        }
        catch (...) {
            dflError("Unknown exception");
            failure = true;
        }
    }
    if ( failure || aborted() ) {
        emitFailure();
    }
    else {
        outputSort(0);
        emitSuccess();
    }

}



QByteArray WorkerLoadRaw::convert(const QString &filename)
{
    QString dcraw_executable("dcraw");
    QStringList arguments;
    QProcess dcraw;
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
        //passthrough
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
    /* the RAW photo */
    arguments << filename;

    dcraw.start(dcraw_executable, arguments, QProcess::ReadOnly);
    QByteArray data;
    dcraw.waitForFinished();
    int rc = dcraw.exitCode();
    if ( 0 == rc )
        data = dcraw.readAllStandardOutput();
    dflDebug("dcraw bytes read: %d", data.size());
    return data;
}

void WorkerLoadRaw::setTags(const QString &filename, Photo &photo)
{
    RawInfo info;
    QFileInfo finfo(filename);
    info.probeFile(filename);
    //photo.writeJPG("/tmp/"+finfo.fileName()+".jpg");
    photo.setTag("Name", finfo.fileName());
    photo.setTag("Directory", finfo.dir().path());
    photo.setTag("ISO Speed", QString("%0").arg(info.isoSpeed()));
    photo.setTag("Shutter", QString("%0").arg(info.shutterSpeed()));
    photo.setTag("Aperture", QString("%0").arg(info.aperture()));
    photo.setTag("Focal length", QString("%0").arg(info.focal()));
    photo.setTag("D65 R multiplier", QString("%0").arg(info.daylightMultipliers().r));
    photo.setTag("D65 G multiplier", QString("%0").arg(info.daylightMultipliers().g));
    photo.setTag("D65 B multiplier", QString("%0").arg(info.daylightMultipliers().b));
    photo.setTag("Camera", info.camera());
    photo.setTag("TimeStamp", info.timestamp());
    photo.setTag("Filter pattern", info.filterPattern());
    photo.setTag("Color Space", m_loadraw->getColorSpace());
    photo.setTag("Debayer", m_loadraw->getDebayer());
    photo.setTag("White Balance", m_loadraw->getWhiteBalance());
    if ( m_loadraw->m_debayerValue == OpLoadRaw::NoDebayer ) {
        photo.setTag("Pixels", "CFA");
    }
    else {
        photo.setTag("Pixels", "RGB");
    }
}
