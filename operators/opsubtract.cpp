#include "opsubtract.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"
#include "algorithm.h"
#include <Magick++.h>
using Magick::Quantum;


class WorkerSubtract : public OperatorWorker {

public:
    WorkerSubtract(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}

    void subtract(Magick::Image& minuend,
                  Magick::Image& subtrahend,
                  Magick::Image* addend,
                  Magick::Image& underflow) {
        int w = minuend.columns(),
                h = minuend.rows();
        int s_w = subtrahend.columns(),
                s_h = subtrahend.rows();
        if ( (w != s_w ||
              h != s_h) &&
             (s_w !=1 || s_h != 1 )) {
            dflError("size mismatch");
            return;
        }
        minuend.modifyImage();
        underflow.modifyImage();
        Magick::Pixels minuend_cache(minuend);
        Magick::Pixels subtrahend_cache(subtrahend);
        Magick::Pixels *addend_cache = NULL;
        if (addend)
            addend_cache = new Magick::Pixels(*addend);
        Magick::Pixels underflow_cache(underflow);
#pragma omp parallel for
        for ( int y = 0 ; y < h ; ++y ) {
            Magick::PixelPacket *minuend_pixels = minuend_cache.get(0, y, w, 1);
            Magick::PixelPacket *underflow_pixels = underflow_cache.get(0, y, w,1);
            Magick::PixelPacket *addend_pixels = NULL;
            if ( addend_cache )
                addend_pixels = addend_cache->get(0, (1 == s_h ? 0 : y), s_w, 1);
            const Magick::PixelPacket *subtrahend_pixels = subtrahend_cache.getConst(0, (1 == s_h ? 0 : y), s_w, 1);
            if ( !minuend_pixels ) continue;
            if ( !subtrahend_pixels ) continue;
            for ( int x = 0 ; x < w ; ++x ) {
                int s_x = ( 1 == s_w ) ? 0 : x ;
                quantum_t r = minuend_pixels[x].red - subtrahend_pixels[s_x].red;
                quantum_t g = minuend_pixels[x].green - subtrahend_pixels[s_x].green;
                quantum_t b = minuend_pixels[x].blue - subtrahend_pixels[s_x].blue;
                if ( addend_pixels ) {
                    r += addend_pixels[s_x].red;
                    g += addend_pixels[s_x].green;
                    b += addend_pixels[s_x].blue;
                }
                underflow_pixels[x].red = underflow_pixels[x].green = underflow_pixels[x].blue =
                        ( r < 0 || g < 0 || b < 0) ? QuantumRange : 0;
                minuend_pixels[x].red = clamp<quantum_t>(r, 0, QuantumRange);
                minuend_pixels[x].green = clamp<quantum_t>(g, 0, QuantumRange);
                minuend_pixels[x].blue = clamp<quantum_t>(b, 0, QuantumRange);
            }
        }
        minuend_cache.sync();
    }

    void play() {
        Q_ASSERT( m_inputs.count() == 3 );
        if ( m_inputs[1].count() == 0 )
            return OperatorWorker::play();
        int n_photos = m_inputs[0].count() * m_inputs[1].count();
        int n = 0;
        int n_sub = 0;
        foreach(Photo subtrahend, m_inputs[1]) {
            foreach(Photo minuend, m_inputs[0]) {
                ++n;
                if (aborted())
                    continue;
                Photo underflow(minuend);
                Magick::Image *addend_image=NULL;
                Magick::Image *addend_curve=NULL;
                if ( n_sub < m_inputs[2].count() ) {
                    addend_image = &m_inputs[2][n_sub].image();
                    addend_curve = &m_inputs[2][n_sub].curve();
                }
                try {
                    subtract(minuend.image(), subtrahend.image(), addend_image, underflow.image());
                    if ( subtrahend.image().columns() == 1 &&
                         subtrahend.image().rows() == 1 ) {
                        Photo dummy(minuend.curve(),Photo::Linear);
                        subtract(minuend.curve(), subtrahend.image(), addend_curve, dummy.image());
                    }
                }
                catch (std::exception &e) {
                    setError(subtrahend, e.what());
                    setError(minuend, e.what());
                    if (addend_image)
                        setError(m_inputs[2][n_sub], e.what());
                    emitFailure();
                    return;
                }

                outputPush(0, minuend);
                outputPush(1, underflow);
                emit progress(n, n_photos);
            }
            ++n_sub;
        }
        if ( aborted() )
            emitFailure();
        else
            emitSuccess();
    }

    Photo process(const Photo &photo, int, int) { return Photo(photo); }
};


OpSubtract::OpSubtract(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, "Subtract", Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Minuend","Minuend",OperatorInput::Set, this));
    addInput(new OperatorInput("Subtrahend","Subtrahend",OperatorInput::Set, this));
    addInput(new OperatorInput("Addend","Addend",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Difference", "Difference", this));
    addOutput(new OperatorOutput("Underflow", "Underflow", this));

}

OpSubtract *OpSubtract::newInstance()
{
    return new OpSubtract(m_process);
}

OperatorWorker *OpSubtract::newWorker()
{
    return new WorkerSubtract(m_thread, this);
}
