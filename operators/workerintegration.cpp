#include "workerintegration.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "algorithm.h"
#include <Magick++.h>

using Magick::Quantum;

WorkerIntegration::WorkerIntegration(OpIntegration::RejectionType rejectionType,
                                     qreal upper,
                                     qreal lower,
                                     OpIntegration::NormalizationType normalizationType,
                                     qreal customNormalizationValue,
                                     QThread *thread,
                                     OpIntegration *op) :
    OperatorWorker(thread, op),
    m_rejectionType(rejectionType),
    m_upper(upper),
    m_lower(lower),
    m_normalizationType(normalizationType),
    m_customNormalizationValue(customNormalizationValue),
    m_integrationPlane(0),
    m_countPlane(0),
    m_w(0),
    m_h(0)
{
}

WorkerIntegration::~WorkerIntegration()
{
    delete[] m_integrationPlane;
    delete[] m_countPlane;
}

bool WorkerIntegration::play_onInput(int idx)
{
    Q_ASSERT( idx == 0 );
    int photoCount = 0;
    int photoN = 0;
    foreach(OperatorOutput *output, m_operator->m_inputs[0]->sources()) {
        photoCount+=output->m_result.count();
    }
    foreach(OperatorOutput *output, m_operator->m_inputs[0]->sources()) {
        foreach(Photo photo, output->m_result) {
            if ( aborted() ) {
                emitFailure();
                return false;
            }
            Magick::Image& image = photo.image();
            if ( ! m_integrationPlane ) {
                createPlanes(image);
            }

            Magick::Pixels pixel_cache(image);
            int line = 0;
#pragma omp parallel for
            for ( int y = 0 ; y < m_h ; ++y ) {
                Magick::PixelPacket *pixels = pixel_cache.get(0, y, m_w, 1);
                if ( !pixels ) continue;
                for ( int x = 0 ; x < m_w ; ++x ) {
                    m_integrationPlane[y*m_w*3+x*3+0] += pixels[x].red;
                    m_integrationPlane[y*m_w*3+x*3+1] += pixels[x].green;
                    m_integrationPlane[y*m_w*3+x*3+2] += pixels[x].blue;
                    ++m_countPlane[y*m_w*3+x*3+0];
                    ++m_countPlane[y*m_w*3+x*3+1];
                    ++m_countPlane[y*m_w*3+x*3+2];
                }
#pragma omp critical
                {
                    ++line;
                    if ( 0 == line % 100 )
                        emitProgress(photoN, photoCount, line, m_h);
                }
            }
#pragma omp barrier
            ++photoN;
        }
    }
    Photo newPhoto;
    newPhoto.create(m_w, m_h);
    Magick::Image& newImage = newPhoto.image();
    newImage.modifyImage();
    Magick::Pixels pixel_cache(newImage);
    qreal mul = ( m_normalizationType == OpIntegration::Custom ? m_customNormalizationValue : 1. );
#pragma omp parallel for
    for ( int y = 0 ; y < m_h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0, y, m_w, 1);
        for ( int x = 0 ; x < m_w ; ++x ) {
            pixels[x].red   =
                    clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+0]/m_countPlane[y*m_w*3+x*3+0], 0, QuantumRange);
            pixels[x].green =
                    clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+1]/m_countPlane[y*m_w*3+x*3+1], 0, QuantumRange);
            pixels[x].blue  =
                    clamp<quantum_t>(mul*m_integrationPlane[y*m_w*3+x*3+2]/m_countPlane[y*m_w*3+x*3+2], 0, QuantumRange);
        }
    }
#pragma omp barrier
    newPhoto.setTag("Name", "Integration");
    m_operator->m_outputs[0]->m_result.push_back(newPhoto);
    emitSuccess();
    return true;
}

void WorkerIntegration::createPlanes(Magick::Image &image)
{
    m_w = image.columns();
    m_h = image.rows();
    m_integrationPlane = new quantum_t[m_w*m_h*3];
    m_countPlane = new int[m_w*m_h*3];
    for ( int i = 0 ; i < m_w*m_h ; ++i ) {
        m_integrationPlane[i] = m_countPlane[i] = 0;
    }
}
