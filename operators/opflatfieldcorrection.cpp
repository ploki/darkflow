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
        Q_ASSERT(m_inputs.count() == 2);
        foreach(Photo photo, m_inputs[1]) {
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

    void correct(Magick::Image &image,
                 Magick::Image& flatfield,
                 Magick::Image& overflow,
                 Triplet& max) {
        int w = image.columns();
        int h = image.rows();
        int f_w = flatfield.columns();
        int f_h = flatfield.rows();
        if ( w != f_w || h != f_h ) {
            qWarning("flatfiled: size mismatch");
            return;
        }
        image.modifyImage();
        overflow.modifyImage();
        Magick::Pixels image_cache(image);
        Magick::Pixels flatfield_cache(flatfield);
        Magick::Pixels overflow_cache(overflow);
#pragma omp parallel for
        for ( int y = 0 ; y < h ; ++y ) {
            Magick::PixelPacket *image_pixels = image_cache.get(0, y, w, 1);
            Magick::PixelPacket *overflow_pixels = overflow_cache.get(0, y, w, 1);
            const Magick::PixelPacket *flatfield_pixels = flatfield_cache.getConst(0, y, w, 1);
            if ( !image_pixels ) continue;
            if ( !flatfield_pixels ) continue;
            for ( int x = 0 ; x < w ; ++x ) {
                Triplet ff;
                bool singularity = false;
                if ( flatfield_pixels[x].red )
                    ff.r = flatfield_pixels[x].red;
                else {
                    //continue;
                    ff.r = 1;
                    singularity = true;
                }
                if ( flatfield_pixels[x].green )
                    ff.g = flatfield_pixels[x].green;
                else {
                    //continue;
                    ff.g = 1;
                    singularity = true;
                }
                if ( flatfield_pixels[x].blue )
                    ff.b = flatfield_pixels[x].blue;
                else {
                    //continue;
                    ff.b = 1;
                    singularity = true;
                }
                quantum_t r = quantum_t(image_pixels[x].red) * max.r / ff.r;
                quantum_t g = quantum_t(image_pixels[x].green) * max.g / ff.g;
                quantum_t b = quantum_t(image_pixels[x].blue) * max.b / ff.b;
                overflow_pixels[x].red = overflow_pixels[x].green = overflow_pixels[x].blue =
                        ( singularity || r > QuantumRange || g > QuantumRange || b > QuantumRange )
                        ? QuantumRange
                        : 0;

                image_pixels[x].red =
                        clamp<quantum_t>(r, 0, QuantumRange);
                image_pixels[x].green =
                        clamp<quantum_t>(g, 0, QuantumRange);
                image_pixels[x].blue =
                        clamp<quantum_t>(b, 0, QuantumRange);
            }
        }
#pragma omp barrier
    }
    void play(QVector<QVector<Photo> > inputs, int n_outputs) {
        Q_ASSERT( inputs.count() == 2 );
        if ( inputs[1].count() == 0 )
            return OperatorWorker::play(inputs, n_outputs);
        m_inputs = inputs;
        play_prepareOutputs(n_outputs);
        play_analyseSources();
        int n_photos = inputs[0].count() * inputs[1].count();
        int n = 0;
        int source_flatfield_idx = 0;
        foreach(Photo flatfield, inputs[1]) {
            foreach(Photo photo, inputs[0]) {
                ++n;
                if (aborted())
                    continue;
                Photo overflow(photo);
                correct(photo.image(), flatfield.image(),
                        overflow.image(), m_max[source_flatfield_idx]);
                m_outputs[0].push_back(photo);
                m_outputs[1].push_back(overflow);
                emit progress(n, n_photos);
            }
            ++source_flatfield_idx;
        }
        if ( aborted() )
            emitFailure();
        else
            emitSuccess();
    }

    Photo process(const Photo &photo, int, int) { return Photo(photo); }
private:
    QVector<Triplet> m_max;
};

OpFlatFieldCorrection::OpFlatFieldCorrection(Process *parent) :
    Operator(OP_SECTION_BLEND, "Flat-Field Correction", parent)
{
    m_inputs.push_back(new OperatorInput("Uneven images","Uneven images",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Flat-field","Flat-field",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Flattened", "Flattened", this));
    m_outputs.push_back(new OperatorOutput("Overflow", "Overflow", this));
}

OpFlatFieldCorrection *OpFlatFieldCorrection::newInstance()
{
    return new OpFlatFieldCorrection(m_process);
}

OperatorWorker *OpFlatFieldCorrection::newWorker()
{
    return new WorkerFlatField(m_thread, this);
}
