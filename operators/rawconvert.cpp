#include <QStringList>
#include <QProcess>
#include <QByteArray>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include "process.h"

#include <Magick++.h>

#include "rawconvert.h"
#include "rawinfo.h"
#include "operatoroutput.h"
#include "operatorloadraw.h"
#include "photo.h"

RawConvert::RawConvert(QThread *thread, OperatorLoadRaw *op) :
    OperatorWorker(thread, op),
    m_loadraw(op)
{}

void RawConvert::play()
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
        Magick::Blob blob(data.data(),data.length());
        Photo::Gamma gamma;
        switch(m_loadraw->m_colorSpaceValue) {
        default:
        case OperatorLoadRaw::Linear: gamma = Photo::Linear; break;
        case OperatorLoadRaw::IUT_BT_709: gamma = Photo::IUT_BT_709; break;
        case OperatorLoadRaw::sRGB: gamma = Photo::sRGB; break;
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
    if ( failure ) {
        emitFailure();
    }
    else {
        outputSort(0);
        emitSuccess();
    }

}



QByteArray RawConvert::convert(const QString &filename)
{
    QString dcraw_executable("dcraw");
    QStringList arguments;
    QProcess dcraw;
    switch(m_loadraw->m_whiteBalanceValue ) {
    case OperatorLoadRaw::NoWhiteBalance:
        arguments << "-r" << "1" << "1" << "1" << "1";
        break;
    case OperatorLoadRaw::RawColors:
        arguments << "-o" << "0";
        break;
    case OperatorLoadRaw::Camera:
        arguments << "-w";
        break;
    case OperatorLoadRaw::Daylight:
        //it is the default
        break;
    }
    switch(m_loadraw->m_debayerValue) {
    case OperatorLoadRaw::NoDebayer:
        arguments << "-d";
        break;
    case OperatorLoadRaw::HalfSize:
        arguments << "-h";
        break;
    case OperatorLoadRaw::Low:
        arguments << "-q" << "0";
        break;
    case OperatorLoadRaw::VNG:
        arguments << "-q" << "1";
        break;
    case OperatorLoadRaw::PPG:
        arguments << "-q" << "2";
        break;
    case OperatorLoadRaw::AHD:
        arguments << "-q" << "3";
        break;
    }
    switch(m_loadraw->m_colorSpaceValue) {
    case OperatorLoadRaw::Linear:
        arguments << "-4";
        break;
    case OperatorLoadRaw::sRGB:
        arguments << "-g" << "2.4" << "12.92";
        //passthrough
    case OperatorLoadRaw::IUT_BT_709:
        arguments << "-6";
        break;
    }
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
    data = dcraw.readAllStandardOutput();
    return data;
}

void RawConvert::setTags(const QString &filename, Photo &photo)
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
    if ( m_loadraw->m_debayerValue == OperatorLoadRaw::NoDebayer ) {
        photo.setTag("Pixels", "CFA");
    }
    else {
        photo.setTag("Pixels", "RGB");
    }
}
