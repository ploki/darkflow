#include "opflatfieldcorrection.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include <Magick++.h>
#include "algorithm.h"

using Magick::Quantum;

struct Triplet {
    Triplet() : r(0), g(0), b(0) {}
    quantum_t r;
    quantum_t g;
    quantum_t b;
};

class WorkerFlatField : public OperatorWorker {
public:
    WorkerFlatField(QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_max()
    {}
    void play_analyseSources() {
        for ( int i = 1,
              s = m_operator->m_inputs.count() ;
              i < s ;
              ++i) {
            foreach(OperatorOutput *output, m_operator->m_inputs[i]->sources()) {
                foreach(Photo photo, output->m_result) {
                    Magick::Image& image = photo.image();
                    Magick::Pixels pixels_cache(image);
                    int w = image.columns();
                    int h = image.rows();
                    Triplet max;
                    for ( int y = 0 ; y < h ; ++ y) {
                        Magick::PixelPacket * pixels = pixels_cache.get(0, y, w, 1);
                        for ( int x = 0 ; x < w ; ++x ) {
                            if ( pixels[x].red > max.r )
                                max.r = pixels[x].red;
                            if ( pixels[x].green > max.g )
                                max.g = pixels[x].green;
                            if ( pixels[x].blue > max.b )
                                max.b = pixels[x].blue;
                        }
                    }
                    m_max.push_back(max);
                }
            }
        }
    }
    void correct(Magick::Image &image, Magick::Image& flatfield, Triplet& max) {
        int w = image.columns();
        int h = image.rows();
        int f_w = flatfield.columns();
        int f_h = flatfield.rows();
        if ( w != f_w || h != f_h ) {
            qWarning("flatfiled: size mismatch");
            return;
        }
        image.modifyImage();
        Magick::Pixels image_cache(image);
        Magick::Pixels flatfield_cache(flatfield);
#pragma omp parallel for
        for ( int y = 0 ; y < h ; ++y ) {
            Magick::PixelPacket *image_pixels = image_cache.get(0, y, w, 1);
            const Magick::PixelPacket *flatfield_pixels = flatfield_cache.getConst(0, y, w, 1);
            if ( !image_pixels ) continue;
            if ( !flatfield_pixels ) continue;
            for ( int x = 0 ; x < w ; ++x ) {
                Triplet ff;
                if ( flatfield_pixels[x].red )
                    ff.r = flatfield_pixels[x].red;
                else {
                    continue;
                    ff.r = 1;
                }
                if ( flatfield_pixels[x].green )
                    ff.g = flatfield_pixels[x].green;
                else {
                    continue;
                    ff.g = 1;
                }
                if ( flatfield_pixels[x].blue )
                    ff.b = flatfield_pixels[x].blue;
                else {
                    continue;
                    ff.b = 1;
                }
                image_pixels[x].red =
                        clamp<quantum_t>(quantum_t(image_pixels[x].red) * max.r / ff.r, 0, QuantumRange);
                image_pixels[x].green =
                        clamp<quantum_t>(quantum_t(image_pixels[x].green) * max.g / ff.g, 0, QuantumRange);
                image_pixels[x].blue =
                        clamp<quantum_t>(quantum_t(image_pixels[x].blue) * max.b / ff.b, 0, QuantumRange);
            }
        }
#pragma omp barrier
    }

    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        int source_flatfield_idx = 0;
        for ( int i = 1,
              s = m_operator->m_inputs.count() ;
              i < s ;
              ++i) {
            foreach(OperatorOutput *output, m_operator->m_inputs[i]->sources()) {
                foreach(Photo flatfield, output->m_result) {
                    correct(newPhoto.image(), flatfield.image(), m_max[source_flatfield_idx++]);
                }
            }
        }
        return newPhoto;
    }
private:
    QVector<Triplet> m_max;
};

OpFlatFieldCorrection::OpFlatFieldCorrection(Process *parent) :
    Operator(OP_SECTION_BLEND, "Flat-Field Correction", parent)
{
    m_inputs.push_back(new OperatorInput("Uneven images","Uneven images",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Flat-field","Flat-field",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Flattened", "Flattened", this));
}

OpFlatFieldCorrection *OpFlatFieldCorrection::newInstance()
{
    return new OpFlatFieldCorrection(m_process);
}

OperatorWorker *OpFlatFieldCorrection::newWorker()
{
    return new WorkerFlatField(m_thread, this);
}
