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
#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <Magick++.h>
#include <memory>

#include "ports.h"
#include "preferences.h"

#if defined(DFL_USE_GCD)
#include <QMutex>
#include <QMutexLocker>
#include <QSemaphore>
#include "console.h"
#endif

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

class AtWork {
public:
    AtWork() { preferences->incrAtWork(); }
    ~AtWork() { preferences->decrAtWork(); }
};


#if defined(DFL_USE_GCD)

class DflDispatch {
    QMutex m_mutex;
    QSemaphore sem;
    bool do_release;
    dispatch_queue_t m_queue;
public:
    DflDispatch(int numThreads) :
        m_mutex(),
        sem(numThreads),
#ifdef DISPATCH_QUEUE_CONCURRENT
        do_release(true),
        m_queue(dispatch_queue_create("org.darkflow.fakeOmp", DISPATCH_QUEUE_CONCURRENT))
#else
        do_release(false),
        m_queue(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0))
#endif
    {
        if (0 == m_queue) {
            dflWarning(QObject::tr("Failed to create dispatch queue, going mono thread"));
            m_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            sem.acquire(numThreads-1);
            do_release = false;
        }
    }
    dispatch_queue_t queue() { return m_queue; }
    void acquire() { sem.acquire(1); }
    void release() { sem.release(1); }
    QMutex *mutex() { return &m_mutex; }
    ~DflDispatch() {
        if (do_release)
            dispatch_release(m_queue);
    }
};
class Acquire {
    std::shared_ptr<DflDispatch> m_dispatch;
public:
    Acquire(const std::shared_ptr<DflDispatch>& dispatch) :
        m_dispatch(dispatch) {
        m_dispatch->acquire();
    }
    ~Acquire() {
        m_dispatch->release();
    }
};

#define dfl_threads(chunk, ...) \
    schedule(static, chunk) num_threads(OnDiskCache(__VA_ARGS__)?1:DfThreadLimit())

#define dfl_block __block

#define dfl_parallel_for(__var__, __start__, __end__, __stride__, __image_list__, ...) \
do \
{\
    size_t _dfl_start = __start__; \
    size_t _dfl_end = __end__; \
    size_t _dfl_stride = __stride__; \
    size_t _dfl_n_strides = (_dfl_end-_dfl_start)/_dfl_stride; \
    int _dfl_num_threads = (OnDiskCache __image_list__)?1:DfThreadLimit(); \
    std::shared_ptr<DflDispatch> _dfl_dispatch(new DflDispatch(_dfl_num_threads)); \
    if (_dfl_num_threads > 1 ) { \
        dispatch_apply(_dfl_n_strides, \
                   _dfl_dispatch->queue(), \
                   ^(size_t _dfl_idx) { \
                       AtWork atWork; \
                       Acquire sem(_dfl_dispatch); \
                       size_t i_start = _dfl_idx * _dfl_stride + _dfl_start; \
                       size_t i_end = i_start + _dfl_stride; \
                       for ( int __var__ = i_start ; __var__ < (int)i_end ; ++__var__) \
                       { \
                        __VA_ARGS__ \
                       } \
                   }); \
    } else { \
        _dfl_n_strides = _dfl_stride = 0; \
    } \
    for ( int __var__ = _dfl_n_strides*_dfl_stride + _dfl_start ; __var__ < (int)_dfl_end ; ++__var__) \
        { AtWork atWork; \
            { __VA_ARGS__ } \
        } \
} while (0)

#define dfl_critical_section(...) do { \
    QMutexLocker lock(_dfl_dispatch->mutex()); \
    __VA_ARGS__ \
    } while (0)
#define dfl_block_array(type, name, size) type _block_kludge_ ## name[size] = {}; __block type * name = &_block_kludge_ ## name[0]
#else
#define dfl_block_array(type, name, size) type name[size] = {}
#define dfl_block volatile
#define dfl_threads(chunk, ...) schedule(static, chunk) num_threads(OnDiskCache(__VA_ARGS__)?1:DfThreadLimit())
//"num_threads(OnDiskCache " STRINGIFY(__image_list__) " ?1:DfThreadLimit())"
#define STRINGIFY(a) #a
#define dfl_parallel_for(__var__, __start__, __end__, __stride__, __image_list__, ...) \
do {\
    _Pragma(STRINGIFY(omp parallel for schedule(static, __stride__) num_threads((OnDiskCache __image_list__ )?1:DfThreadLimit()))) \
    for(int __var__ = __start__ ; __var__ < __end__ ; ++__var__ ) \
        { AtWork atWork; { __VA_ARGS__ } }\
} while (0)

#define dfl_critical_section(...) _Pragma("omp critical") { __VA_ARGS__ }


#endif



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
    bool saveImage(const QString& filename, const QString &magick, double gamma, double x0, double exposureBoost);

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
