#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <Magick++.h>

/* macros defined in cc command line by pkg-config */
#ifdef QuantumRange
# ifdef MAGICKCORE_HDRI_ENABLE
Q_STATIC_ASSERT(MAGICKCORE_HDRI_ENABLE == 0);
# endif
# ifdef MAGICKCORE_QUANTUM_DEPTH
Q_STATIC_ASSERT(MAGICKCORE_QUANTUM_DEPTH == 16);
# endif
#else
#define USING_GRAPHICSMAGICK
#define QuantumRange (Quantum(65535))
#endif

typedef int quantum_t;

void ResetImage(Magick::Image &image);
bool OnDiskCache();
bool OnDiskCache(const Magick::Image& image);
bool OnDiskCache(const Magick::Image& image1, const Magick::Image& image2);
bool OnDiskCache(const Magick::Image& image1, const Magick::Image& image2, const Magick::Image& image3);
bool OnDiskCache(const Magick::Image& image1, const Magick::Image& image2, const Magick::Image& image3, const Magick::Image& image4);
bool OnDiskCache(const Magick::Image& image1, const Magick::Image& image2, const Magick::Image& image3, const Magick::Image& image4, const Magick::Image& image5);
bool OnDiskCache(const Magick::Image& image1, const Magick::Image& image2, const Magick::Image& image3, const Magick::Image& image4, const Magick::Image& image5, const Magick::Image& image6);
int DfThreadLimit();

#define dfl_threads(chunk, ...) schedule(static, chunk) num_threads(OnDiskCache(__VA_ARGS__)?1:DfThreadLimit())

template<typename T>
class Triplet {
public:
    Triplet() : red(0), green(0), blue(0) {}
    Triplet(T r, T g, T b) :
        red(r), green(g), blue(b) {}
    Triplet(const Triplet& t) :
        red(t.red), green(t.green), blue(t.blue) {}
    Triplet& operator=(const Triplet& t) {
        red=t.red;
        green=t.green;
        blue=t.blue;
        return *this;
    }
    T red;
    T green;
    T blue;
};

class QRectF;

class Photo : public QObject
{
    Q_OBJECT
public:    
    typedef enum {
        Linear     = 1,
        NonLinear  = 2,
        Sqrt       = 4|NonLinear,
        IUT_BT_709 = 8|NonLinear,
        sRGB       = 16|NonLinear,
        HDR        = 32,
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

    QVector<qreal> pixelColor(unsigned x, unsigned y);
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
    void setPoints(const QVector<QPointF>& vec);
    QRectF getROI() const;
    void setROI(const QRectF& rect);
    void setScale(Gamma gamma);
    Gamma getScale() const;

private:
    Magick::Image m_image;
    Magick::Image m_curve;
    Status m_status;
    QMap<QString, QString> m_tags;
    QString m_identity;
    int m_sequenceNumber;


    static Magick::Image newCurve(Gamma gamma);
};

#define TAG_NAME "Name"
#define TAG_DIRECTORY "Directory"
#define TAG_ISO_SPEED "ISO Speed"
#define TAG_SHUTTER "Shutter"
#define TAG_APERTURE "Aperture"
#define TAG_FOCAL_LENGTH "Focal length"
#define TAG_D65_R_MULTIPLIER "D65 R multiplier"
#define TAG_D65_G_MULTIPLIER "D65 G multiplier"
#define TAG_D65_B_MULTIPLIER "D65 B multiplier"
#define TAG_CAMERA "Camera"
#define TAG_TIMESTAMP "TimeStamp"
#define TAG_FILTER_PATTERN "Filter pattern"
#define TAG_COLOR_SPACE "Color Space"
#define TAG_DEBAYER "Debayer"
#define TAG_WHITE_BALANCE "White Balance"
#define TAG_PIXELS "Pixels"
#define TAG_PIXELS_CFA "CFA"
#define TAG_PIXELS_RGB "RGB"
#define TAG_TREAT "Treat"
#define TAG_TREAT_REGULAR ""
#define TAG_TREAT_REFERENCE "Reference"
#define TAG_TREAT_DISCARDED "Discarded"
#define TAG_TREAT_ERROR "ERROR"
#define TAG_POINTS "Points"
#define TAG_ROI "ROI"
#define TAG_HDR_COMP "HDR compensation"
#define TAG_HDR_HIGH "HDR high threshold"
#define TAG_HDR_LOW "HDR low threshold"
#define TAG_SCALE "Scale"
#define TAG_SCALE_LINEAR "Linear"
#define TAG_SCALE_NONLINEAR "Non-linear"
#define TAG_SCALE_HDR "HDR"
#endif // IMAGE_H
