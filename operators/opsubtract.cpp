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

    void subtract(Magick::Image& minuend, Magick::Image& subtrahend) {
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
        Magick::Pixels minuend_cache(minuend);
        Magick::Pixels subtrahend_cache(subtrahend);
#pragma omp parallel for
        for ( int y = 0 ; y < h ; ++y ) {
            Magick::PixelPacket *minuend_pixels = minuend_cache.get(0, y, w, 1);
            const Magick::PixelPacket *subtrahend_pixels = subtrahend_cache.getConst(0, (1 == s_h ? 0 : y), s_w, 1);
            if ( !minuend_pixels ) continue;
            if ( !subtrahend_pixels ) continue;
            for ( int x = 0 ; x < w ; ++x ) {
                int s_x = ( 1 == s_w ) ? 0 : x ;
                minuend_pixels[x].red = clamp<quantum_t>(minuend_pixels[x].red - subtrahend_pixels[s_x].red, 0, QuantumRange);
                minuend_pixels[x].green = clamp<quantum_t>(minuend_pixels[x].green - subtrahend_pixels[s_x].green, 0, QuantumRange);
                minuend_pixels[x].blue = clamp<quantum_t>(minuend_pixels[x].blue - subtrahend_pixels[s_x].blue, 0, QuantumRange);
            }
        }
#pragma omp barrier
        minuend_cache.sync();
    }

    Photo process(const Photo &photo, int, int) {

        Photo newPhoto(photo);
        for ( int i = 1,
              s = m_operator->m_inputs.count() ;
              i < s ;
              ++i ) {
            foreach(OperatorOutput *parentOutput, m_operator->m_inputs[i]->sources()) {
                foreach(Photo subtrahend, parentOutput->m_result) {
                    subtract(newPhoto.image(), subtrahend.image());
                }
            }
        }
        return newPhoto;
    }
};


OpSubtract::OpSubtract(Process *parent) :
    Operator(OP_SECTION_BLEND, "Subtract", parent)
{
    m_inputs.push_back(new OperatorInput("Minuend","Minuend",OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Subtrahend","Subtrahend",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Difference", "Difference", this));

}

OpSubtract *OpSubtract::newInstance()
{
    return new OpSubtract(m_process);
}

OperatorWorker *OpSubtract::newWorker()
{
    return new WorkerSubtract(m_thread, this);
}
