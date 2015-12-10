#include <QVector>
#include <QPointF>

#include "workerlimereg.h"
#include <limereg.h>
#include "photo.h"
#include <Magick++.h>
#include <algorithm.h>

using Magick::Quantum;

WorkerLimereg::WorkerLimereg(qreal maxRotation, qreal maxTranslation, QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_maxRotation(maxRotation),
    m_maxTranslation(maxTranslation),
    m_refIdx(0)
{
}

Photo WorkerLimereg::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLimereg::play_analyseSources()
{
    for (int i = 0, s = m_inputs[0].count() ;
            i < s ;
            ++i ) {
        if ( m_inputs[0][i].getTag("TREAT") == "REFERENCE" ) {
            m_refIdx=i;
            break;
        }
    }
}


static void
ImageToLimereg_Image(Magick::Image& image, Limereg_Image& dst)
{
    dst.pixelType = Limereg_Image::Limereg_Grayscale_8;
    dst.pyramidImage = Limereg_Image::Limereg_NotPyramidized;
    dst.imageWidth = image.columns();
    dst.imageHeight = image.rows();
    dst.pixelBuffer = (unsigned char*)malloc(dst.imageWidth*dst.imageHeight);

    Magick::Pixels cache(image);
    const Magick::PixelPacket *pxl = cache.getConst(0, 0, dst.imageWidth, dst.imageHeight);
#pragma omp parallel for
    for ( unsigned y = 0 ; y < dst.imageHeight ; ++y ) {
        for ( unsigned x = 0 ; x < dst.imageWidth ; ++x ) {
            dst.pixelBuffer[y*dst.imageWidth+x] =
                    clamp<int>(
                      /* pow( */ ((.2126*pxl[dst.imageWidth+x].red +
                     .7152*pxl[y*dst.imageWidth+x].green +
                     .0722*pxl[y*dst.imageWidth+x].blue)/QuantumRange) /* , 1) */ * 256
                    , 16, 255) ;
        }
    }

}
static void
freeLimereg_Image(Limereg_Image& dst) {
    free(dst.pixelBuffer);
}

bool WorkerLimereg::registerLimereg(Limereg_Image &lref,  Limereg_Image& limg, Limereg_TrafoParams& result)
{
    Limereg_RetCode ret;
    Limereg_TrafoLimits limits = {};
    limits.maxRotationDeg = m_maxRotation;
    limits.maxTranslationPercent = m_maxTranslation;
    Limereg_AdvancedRegControl adv = {};
    //    adv.maxIterations = 1000;
    //    adv.stopSensitivity = 0.1;
    //adv.skipFineLevelCount=16;
    double distanceMeasure;
    unsigned int iterationAmount;
    unsigned int iterationsPerLevel;

    ret = Limereg_RegisterImage(&lref, &limg,
                                &limits, Limereg_Trafo_Rigid, &adv,
                                &result, &distanceMeasure,
                                &iterationAmount, &iterationsPerLevel);
    if ( ret == LIMEREG_RET_SUCCESS )
        return true;
    qDebug("failed with code %d", ret);
    return false;
}



bool WorkerLimereg::play_onInput(int idx)
{
    Q_ASSERT(idx == 0);
    Photo reference(m_inputs[0][m_refIdx]);
    Limereg_Image lref;
    ImageToLimereg_Image(reference.image(), lref);

    for (int i = 0, s = m_inputs[0].count() ;
         i < s ;
         ++i ) {
        Photo photo = m_inputs[0][i];
        bool ok=true;
        Limereg_TrafoParams result = {};
        if ( i == m_refIdx ) {
            result.rotationDeg = 0;
            result.xShift = 0;
            result.yShift = 0;
        }
        else {
            Limereg_Image limg = {};
            ImageToLimereg_Image(photo.image(), limg);
            ok = registerLimereg(lref, limg, result);
            freeLimereg_Image(limg);
        }
        if ( ok ) {
            QString points = QString::number(result.xShift + photo.image().columns()/2 )+
                    "," + QString::number(result.yShift + photo.image().rows()/2 );
            QString rot = QString::number(result.rotationDeg);
            photo.setTag("POINTS", points);
            photo.setTag("ROTATION", rot);
        }
        outputPush(0, photo);
        emitProgress(i, s, 0, 1);
    }
    freeLimereg_Image(lref);
    emitSuccess();
    return true;
}
