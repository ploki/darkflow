#ifndef RAWINFO_H
#define RAWINFO_H

#include <QObject>

class RawInfo : public QObject
{
    Q_OBJECT
public:
    struct Multipliers {
        Multipliers();
        qreal r, g, b;
    };

    explicit RawInfo(QObject *parent = 0);

    qreal isoSpeed() const;
    qreal shutterSpeed() const;
    qreal aperture() const;
    qreal focal() const;
    Multipliers daylightMultipliers() const;
    Multipliers cameraMultipliers() const;
    QString camera() const;
    QString timestamp() const;
    QString filterPattern() const;

    bool probeFile(const QString& filename);

signals:

public slots:

private:
    qreal m_isoSpeed;
    qreal m_shutterSpeed;
    qreal m_aperture;
    qreal m_focal;
    Multipliers m_daylightMultipliers;
    Multipliers m_cameraMultipliers;
    QString m_camera;
    QString m_timestamp;
    QString m_filterPattern;

};

#endif // RAWINFO_H
