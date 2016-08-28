#ifndef TRANSFORMVIEW_H
#define TRANSFORMVIEW_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QTransform>

#include "photo.h"
#include <Magick++.h>

class TransformView : public QObject
{
    Q_OBJECT

    Photo m_photo;
    QTransform m_transform;
    int m_w;
    int m_h;
    Ordinary::Pixels *m_cache;
    const Magick::PixelPacket *m_pixels;
    bool m_error;
    bool m_hdr;

public:
    TransformView(const Photo& photo, qreal scale, QVector<QPointF> reference, QObject *parent = 0);
    ~TransformView();

    QRectF boundingBox();
    bool inError();
    bool loadPixels();

    void map(qreal x, qreal y, qreal *tx, qreal *ty);
    Magick::PixelPacket getPixel(int x, int y, bool *definedp);

};

#endif // TRANSFORMVIEW_H
