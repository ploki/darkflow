#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <Magick++.h>

/* macros defined in cc command line by pkg-config */
#ifdef QuantumRange
Q_STATIC_ASSERT(MAGICKCORE_HDRI_ENABLE == 0);
Q_STATIC_ASSERT(MAGICKCORE_QUANTUM_DEPTH == 16);
#else
#define USING_GRAPHICSMAGICK
#define QuantumRange (Quantum(65535))
#endif

typedef int quantum_t;


class Photo : public QObject
{
    Q_OBJECT
public:    
    typedef enum {
        Linear,
        Sqrt,
        IUT_BT_709,
        sRGB,
    } Gamma;
    typedef enum {
        sRGB_EV,
        sRGB_Level,
        Log2,
    } CurveView;
    typedef enum {
        HistogramLinear,
        HistogramLogarithmic,
    } HistogramScale;
    typedef enum {
        HistogramLines,
        HistogramSurfaces,
    } HistogramGeometry;
    typedef enum {
        Undefined,
        Identified,
        Complete
    } Status;
    Photo(Gamma gamma = Linear, QObject *parent = 0);
    Photo(const Magick::Image& image, Gamma gamma, QObject *parent = 0);
    Photo(const Magick::Blob& blob, Gamma gamma, QObject *parent = 0);
    Photo(const Photo& photo);

    ~Photo();

    Photo& operator=(const Photo& photo);

    bool load(const QString& filename);
    bool save(const QString& filename, const QString &magick);

    void createImage(long width, long height);
    void createImageAlike(const Photo& photo);

    const Magick::Image& image() const;
    Magick::Image& image();
    const Magick::Image &curve() const;
    Magick::Image &curve();

    QMap<QString, QString> tags() const;
    void setTag(const QString& name, const QString& value);
    void removeTag(const QString& name);
    QString getTag(const QString& name) const;

    QPixmap imageToPixmap(double gamma, double x0, double exposureBoost);
    QPixmap curveToPixmap(CurveView cv);
    QPixmap histogramToPixmap(HistogramScale scale, HistogramGeometry geometry);
    void writeJPG(const QString& filename);

    int getSequenceNumber() const;
    void setSequenceNumber(int sequenceNumber);
    bool operator<(const Photo &other) const;
    QString getIdentity() const;
    void setIdentity(const QString &identity);

    void setUndefined();
    void setComplete();
    bool isUndefined() const;
    bool isIdentified() const;
    bool isComplete() const;

    QVector<QPointF> getPoints() const;

private:
    Magick::Image m_image;
    Magick::Image m_curve;
    Gamma m_gamma;
    Status m_status;
    QMap<QString, QString> m_tags;
    QString m_identity;
    int m_sequenceNumber;


    static Magick::Image newCurve(Gamma gamma);
};

#endif // IMAGE_H
