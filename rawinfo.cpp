#include <QProcess>
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
    QProcess dcraw;
    dcraw.start(dcraw_executable, arguments, QProcess::ReadOnly|QProcess::Text);
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
