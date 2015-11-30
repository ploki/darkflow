#include "workerblend.h"
#include <Magick++.h>
#include "algorithm.h"

using Magick::Quantum;

WorkerBlend::WorkerBlend(OpBlend::BlendMode mode1, OpBlend::BlendMode mode2, QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_mode1(mode1),
    m_mode2(mode2)
{}

Photo WorkerBlend::process(const Photo &, int, int)
{
    throw 0;
}

static void
blend(quantum_t *rgb, const Magick::PixelPacket *p, OpBlend::BlendMode mode)
{
    switch ( mode ) {
    case OpBlend::Multiply:
        //f(a,b) = a*b
        rgb[0] = (double(rgb[0])/QuantumRange * double(p->red)/QuantumRange) * QuantumRange;
        rgb[1] = (double(rgb[1])/QuantumRange * double(p->green)/QuantumRange) * QuantumRange;
        rgb[2] = (double(rgb[2])/QuantumRange * double(p->blue)/QuantumRange) * QuantumRange;
        break;
    case OpBlend::Screen:
        //f(a,b) = 1-(1-a)(1-b)
        rgb[0] = QuantumRange-(1.-double(rgb[0])/QuantumRange)*(1.-double(p->red)/QuantumRange)*QuantumRange;
        rgb[1] = QuantumRange-(1.-double(rgb[1])/QuantumRange)*(1.-double(p->green)/QuantumRange)*QuantumRange;
        rgb[2] = QuantumRange-(1.-double(rgb[2])/QuantumRange)*(1.-double(p->blue)/QuantumRange)*QuantumRange;
        break;
    case OpBlend::Overlay:
        //f(a,b) = a<.5 ? 2ab : 1-2(1-a)(1-b)
        break;
    case OpBlend::HardLight:
        break;
    case OpBlend::SoftLight:
        break;
    case OpBlend::DivideBrighten: {
        //f(a,b) = a*(1-b)
        double mul[3] = {
            p->red?double(QuantumRange)/double(p->red):0,
            p->green?double(QuantumRange)/double(p->green):0,
            p->blue?double(QuantumRange)/double(p->blue):0
        };
        rgb[0] = mul[0]*rgb[0];
        rgb[1] = mul[1]*rgb[1];
        rgb[2] = mul[2]*rgb[2];
    }
        break;
    case OpBlend::Divide: {
        //f(a,b) = a*(1-b)
        double mul[3] = {
            p->red?double(QuantumRange)/double(p->red):0,
            p->green?double(QuantumRange)/double(p->green):0,
            p->blue?double(QuantumRange)/double(p->blue):0
        };
        double max = 0;
        if ( mul[0] > max) max = mul[0];
        if ( mul[1] > max) max = mul[1];
        if ( mul[2] > max) max = mul[2];
        mul[0]/=max;
        mul[1]/=max;
        mul[2]/=max;
        rgb[0] = mul[0]*rgb[0];
        rgb[1] = mul[1]*rgb[1];
        rgb[2] = mul[2]*rgb[2];
    }
        break;
    case OpBlend::DivideDarken: {
        //f(a,b) = a*(1-b)
        double mul[3] = {
            p->red?1/double(p->red):0,
            p->green?1/double(p->green):0,
            p->blue?1/double(p->blue):0
        };
        rgb[0] = mul[0]*rgb[0];
        rgb[1] = mul[1]*rgb[1];
        rgb[2] = mul[2]*rgb[2];
    }
        break;
    case OpBlend::Addition:
        rgb[0] = rgb[0]+quantum_t(p->red);
        rgb[1] = rgb[1]+quantum_t(p->green);
        rgb[2] = rgb[2]+quantum_t(p->blue);
        break;
    case OpBlend::Subtract:
        rgb[0] = rgb[0]-quantum_t(p->red);
        rgb[1] = rgb[1]-quantum_t(p->green);
        rgb[2] = rgb[2]-quantum_t(p->blue);
        break;
    case OpBlend::Difference:
        break;
    case OpBlend::DarkenOnly:
        rgb[0] = qMin(rgb[0], quantum_t(p->red));
        rgb[1] = qMin(rgb[1], quantum_t(p->green));
        rgb[2] = qMin(rgb[2], quantum_t(p->blue));
        break;
    case OpBlend::LightenOnly:
        rgb[0] = qMax(rgb[0], quantum_t(p->red));
        rgb[1] = qMax(rgb[1], quantum_t(p->green));
        rgb[2] = qMax(rgb[2], quantum_t(p->blue));
        break;
    }
}

static void
blend(Magick::PixelPacket *a,
      const Magick::PixelPacket *b,
      const Magick::PixelPacket *c,
      Magick::PixelPacket *u,
      Magick::PixelPacket *o,
      OpBlend::BlendMode mode1,
      OpBlend::BlendMode mode2)
{
    quantum_t rgb[3] = {0, 0, 0};
    if ( a ) {
        blend(rgb, a, OpBlend::Addition);
    }
    if ( b ) {
        blend(rgb, b, mode1);
    }
    if ( c ) {
        blend(rgb, c, mode2);
    }
    if ( rgb[0] < 0 ) u->red = QuantumRange; else u->red = 0;
    if ( rgb[1] < 0 ) u->green = QuantumRange; else u->green = 0;
    if ( rgb[2] < 0 ) u->blue = QuantumRange; else u->blue = 0;

    if ( rgb[0] > QuantumRange ) o->red = QuantumRange; else o->red = 0;
    if ( rgb[1] > QuantumRange ) o->green = QuantumRange; else o->green = 0;
    if ( rgb[2] > QuantumRange ) o->blue = QuantumRange; else o->blue = 0;

    a->red = clamp(rgb[0]);
    a->green = clamp(rgb[1]);
    a->blue = clamp(rgb[2]);
}

void WorkerBlend::play(QVector<QVector<Photo> > inputs, int n_outputs)
{
    int b_count = inputs[1].count();
    int c_count = inputs[2].count();

    m_inputs = inputs;
    play_prepareOutputs(n_outputs);

    int complete = qMax(b_count, c_count) * inputs[0].count();
    int n = 0;
    foreach(Photo photoA, inputs[0]) {
        if (aborted())
            continue;
        Photo *photoB = NULL;
        Photo *photoC = NULL;
        if ( b_count )
            photoB = &inputs[1][n%b_count];
        if ( c_count )
            photoC = &inputs[2][n%c_count];

        Photo underflow(photoA);
        Photo overflow(photoA);
        Magick::Image *imageA = &photoA.image();
        Magick::Image *imageB = NULL;
        Magick::Image *imageC = NULL;

        imageA->modifyImage();
        underflow.image().modifyImage();
        overflow.image().modifyImage();

        Magick::Pixels *imageA_cache = imageA?new Magick::Pixels(*imageA):NULL;
        Magick::Pixels *imageB_cache = NULL;
        Magick::Pixels *imageC_cache = NULL;
        Magick::Pixels underflow_cache(underflow.image());
        Magick::Pixels overflow_cache(overflow.image());
        int w = imageA->columns();
        int h = imageA->rows();
        int b_w = 0;
        int b_h = 0;
        int c_w = 0;
        int c_h = 0;
        if ( photoB ) {
            imageB = &photoB->image();
            imageB_cache = new Magick::Pixels(*imageB);
            b_w = imageB->columns();
            b_h = imageB->rows();
        }
        if ( photoC ) {
            imageC = &photoC->image();
            imageC_cache = new Magick::Pixels(*imageC);
            c_w = imageC->columns();
            c_h = imageC->rows();
        }
#pragma omp parallel for
        for ( int y = 0 ; y < h ; ++y ) {
            Magick::PixelPacket *pxl_u = underflow_cache.get(0, y, w, 1);
            Magick::PixelPacket *pxl_o = overflow_cache.get(0, y, w, 1);
            Magick::PixelPacket *pxl_A = imageA_cache->get(0, y, w, 1);
            const Magick::PixelPacket *pxl_B = NULL;
            const Magick::PixelPacket *pxl_C = NULL;
            if ( imageB_cache )
                pxl_B = imageB_cache->getConst(0, y%b_h, b_w, 1);
            if ( imageC_cache )
                pxl_C = imageC_cache->getConst(0, y%c_h, c_w, 1);
            for ( int x = 0 ; x < w ; ++x ) {
                const Magick::PixelPacket *pB = NULL;
                const Magick::PixelPacket *pC = NULL;
                if ( pxl_B ) pB = pxl_B+(x%b_w);
                if ( pxl_C ) pC = pxl_C+(x%c_w);
                blend(pxl_A+x,
                      pB, pC,
                      pxl_u+x,
                      pxl_o+x,
                      m_mode1,
                      m_mode2);
            }
        }
        imageA_cache->sync();
        underflow_cache.sync();
        overflow_cache.sync();
        m_outputs[0].push_back(photoA);
        m_outputs[1].push_back(overflow);
        m_outputs[2].push_back(underflow);
        delete imageA_cache;
        delete imageB_cache;
        delete imageC_cache;
        emitProgress(n,complete, 1, 1);
        ++n;
    }
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
}

