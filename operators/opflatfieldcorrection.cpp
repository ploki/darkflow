#include "opflatfieldcorrection.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "photo.h"
#include <Magick++.h>
#include "algorithm.h"
#include "hdr.h"

using Magick::Quantum;

typedef float real;

class WorkerFlatField : public OperatorWorker {
public:
    WorkerFlatField(bool outputHDR, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_outputHDR(outputHDR),
        m_max()
    {}
    void play_analyseSources() {
        Q_ASSERT(m_inputs.count() == 2);
        foreach(Photo photo, m_inputs[1]) {
            bool hdr = photo.getScale() == Photo::HDR;
            try  {
                Magick::Image& image = photo.image();
                Magick::Pixels pixels_cache(image);
                int w = image.columns();
                int h = image.rows();
                Triplet<real> max;
                for ( int y = 0 ; y < h ; ++ y) {
                    const Magick::PixelPacket * pixels = pixels_cache.getConst(0, y, w, 1);
                    for ( int x = 0 ; x < w ; ++x ) {
                        if ( pixels[x].red > max.red )
                            max.red = pixels[x].red;
                        if ( pixels[x].green > max.green )
                            max.green = pixels[x].green;
                        if ( pixels[x].blue > max.blue )
                            max.blue = pixels[x].blue;
                    }
                }
                if ( hdr ) {
                    max.red = fromHDR(max.red);
                    max.green = fromHDR(max.green);
                    max.blue = fromHDR(max.blue);
                }
                m_max.push_back(max);
            }
            catch (std::exception &e) {
                setError(photo, e.what());
                emitFailure();
            }
        }
    }

    void correct(Magick::Image &image, bool imageIsHDR,
                 Magick::Image& flatfield, bool flatfieldIsHDR,
                 Magick::Image& overflow,
                 Triplet<real> & max) {
        int w = image.columns();
        int h = image.rows();
        int f_w = flatfield.columns();
        int f_h = flatfield.rows();
        if ( w != f_w || h != f_h ) {
            dflError("size mismatch");
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
                Triplet<real> ff;
                bool singularity = false;
                if ( flatfield_pixels[x].red )
                    ff.red = flatfield_pixels[x].red;
                else {
                    //continue;
                    ff.red = 1;
                    singularity = true;
                }
                if ( flatfield_pixels[x].green )
                    ff.green = flatfield_pixels[x].green;
                else {
                    //continue;
                    ff.green = 1;
                    singularity = true;
                }
                if ( flatfield_pixels[x].blue )
                    ff.blue = flatfield_pixels[x].blue;
                else {
                    //continue;
                    ff.blue = 1;
                    singularity = true;
                }
                if ( flatfieldIsHDR ) {
                    ff.red = fromHDR(ff.red);
                    ff.green = fromHDR(ff.green);
                    ff.blue = fromHDR(ff.blue);
                }
                real r = image_pixels[x].red;
                real g = image_pixels[x].green;
                real b = image_pixels[x].blue;
                if ( imageIsHDR ) {
                    r = fromHDR(r);
                    g = fromHDR(g);
                    b = fromHDR(b);
                }
                r = r * max.red  / ff.red;
                g = g * max.green / ff.green;
                b = b * max.blue / ff.blue;
                overflow_pixels[x].red = overflow_pixels[x].green = overflow_pixels[x].blue =
                        ( singularity || r > QuantumRange || g > QuantumRange || b > QuantumRange )
                        ? QuantumRange
                        : 0;
                if ( m_outputHDR ) {
                    image_pixels[x].red =
                            clamp<quantum_t>(toHDR(r));
                    image_pixels[x].green =
                            clamp<quantum_t>(toHDR(g));
                    image_pixels[x].blue =
                            clamp<quantum_t>(toHDR(b));
                }
                else {
                    image_pixels[x].red =
                            clamp<quantum_t>(r);
                    image_pixels[x].green =
                            clamp<quantum_t>(g);
                    image_pixels[x].blue =
                            clamp<quantum_t>(b);
                }
            }
        }
    }

    void play() {
        Q_ASSERT( m_inputs.count() == 2 );
        if ( m_inputs[1].count() == 0 )
            return OperatorWorker::play();
        play_analyseSources();
        int n_photos = m_inputs[0].count() * m_inputs[1].count();
        int n = 0;
        int source_flatfield_idx = 0;
        foreach(Photo flatfield, m_inputs[1]) {
            foreach(Photo photo, m_inputs[0]) {
                ++n;
                if (aborted())
                    continue;
                try {
                    Photo overflow(photo);
                    correct(photo.image(), photo.getScale() == Photo::HDR,
                            flatfield.image(), flatfield.getScale() == Photo::HDR,
                            overflow.image(),
                            m_max[source_flatfield_idx]);
                    if ( m_outputHDR )
                        photo.setScale(Photo::HDR);
                    else if ( photo.getScale() == Photo::HDR )
                        photo.setScale(Photo::Linear);
                    overflow.setScale(Photo::Linear);
                    outputPush(0, photo);
                    outputPush(1, overflow);
                    emit progress(n, n_photos);
                }
                catch (std::exception &e) {
                    setError(flatfield, e.what());
                    setError(photo, e.what());
                    emitFailure();
                    return;
                }
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
    bool m_outputHDR;
    QVector<Triplet<real> > m_max;
};

OpFlatFieldCorrection::OpFlatFieldCorrection(Process *parent) :
    Operator(OP_SECTION_BLEND, "Flat-Field Correction", Operator::All, parent),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", "Output HDR", this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    m_outputHDR->addOption("No", false, true);
    m_outputHDR->addOption("Yes", true);

    addInput(new OperatorInput("Uneven images","Uneven images",OperatorInput::Set, this));
    addInput(new OperatorInput("Flat-field","Flat-field",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Flattened", "Flattened", this));
    addOutput(new OperatorOutput("Overflow", "Overflow", this));
    addParameter(m_outputHDR);
}

OpFlatFieldCorrection *OpFlatFieldCorrection::newInstance()
{
    return new OpFlatFieldCorrection(m_process);
}

OperatorWorker *OpFlatFieldCorrection::newWorker()
{
    return new WorkerFlatField(m_outputHDRValue, m_thread, this);
}

void OpFlatFieldCorrection::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}
