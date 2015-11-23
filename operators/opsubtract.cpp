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

    void subtract(Magick::Image& minuend, Magick::Image& subtrahend, Magick::Image& underflow) {
        int w = minuend.columns(),
                h = minuend.rows();
        int s_w = subtrahend.columns(),
                s_h = subtrahend.rows();
        if ( (w != s_w ||
              h != s_h) &&
             (s_w !=1 || s_h != 1 )) {
            qWarning("Subtract: size mismatch");
            return;
        }
        minuend.modifyImage();
        underflow.modifyImage();
        Magick::Pixels minuend_cache(minuend);
        Magick::Pixels subtrahend_cache(subtrahend);
        Magick::Pixels underflow_cache(underflow);
#pragma omp parallel for
        for ( int y = 0 ; y < h ; ++y ) {
            Magick::PixelPacket *minuend_pixels = minuend_cache.get(0, y, w, 1);
            Magick::PixelPacket *underflow_pixels = underflow_cache.get(0, y, w,1);
            const Magick::PixelPacket *subtrahend_pixels = subtrahend_cache.getConst(0, (1 == s_h ? 0 : y), s_w, 1);
            if ( !minuend_pixels ) continue;
            if ( !subtrahend_pixels ) continue;
            for ( int x = 0 ; x < w ; ++x ) {
                int s_x = ( 1 == s_w ) ? 0 : x ;
                quantum_t r = minuend_pixels[x].red - subtrahend_pixels[s_x].red;
                quantum_t g = minuend_pixels[x].green - subtrahend_pixels[s_x].green;
                quantum_t b = minuend_pixels[x].blue - subtrahend_pixels[s_x].blue;
                underflow_pixels[x].red = underflow_pixels[x].green = underflow_pixels[x].blue =
                        ( r < 0 || g < 0 || b < 0) ? QuantumRange : 0;
                minuend_pixels[x].red = clamp<quantum_t>(r, 0, QuantumRange);
                minuend_pixels[x].green = clamp<quantum_t>(g, 0, QuantumRange);
                minuend_pixels[x].blue = clamp<quantum_t>(b, 0, QuantumRange);
            }
        }
#pragma omp barrier
        minuend_cache.sync();
    }
    void play(QVector<QVector<Photo> > inputs, int n_outputs) {
        Q_ASSERT( inputs.count() == 2 );
        if ( inputs[1].count() == 0 )
            return OperatorWorker::play(inputs, n_outputs);
        m_inputs = inputs;
        play_prepareOutputs(n_outputs);
        int n_photos = inputs[0].count() * inputs[1].count();
        int n = 0;
        foreach(Photo subtrahend, inputs[1]) {
            foreach(Photo minuend, inputs[0]) {
                ++n;
                if (aborted())
                    continue;
                Photo underflow(minuend);
                subtract(minuend.image(), subtrahend.image(), underflow.image());
                if ( subtrahend.image().columns() == 1 &&
                     subtrahend.image().rows() == 1 ) {
                    Photo dummy(minuend.curve(),Photo::Linear);
                    subtract(minuend.curve(), subtrahend.image(), dummy.image());
                }
                m_outputs[0].push_back(minuend);
                m_outputs[1].push_back(underflow);
                emit progress(n, n_photos);
            }
        }
        if ( aborted() )
            emitFailure();
        else
            emitSuccess();
    }

    Photo process(const Photo &photo, int, int) { return Photo(photo); }
};


OpSubtract::OpSubtract(Process *parent) :
    Operator(OP_SECTION_BLEND, "Subtract", parent)
{
    m_inputs.push_back(new OperatorInput("Minuend","Minuend",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Subtrahend","Subtrahend",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Difference", "Difference", this));
    m_outputs.push_back(new OperatorOutput("Underflow", "Underflow", this));

}

OpSubtract *OpSubtract::newInstance()
{
    return new OpSubtract(m_process);
}

OperatorWorker *OpSubtract::newWorker()
{
    return new WorkerSubtract(m_thread, this);
}
