#include "opdeconvolution.h"
#include "workerdeconvolution.h"
#include <list>
#include "algorithm.h"

using Magick::Quantum;

WorkerDeconvolution::WorkerDeconvolution(QThread *thread, OpDeconvolution *op) :
    OperatorWorker(thread, op)
{
}

Photo WorkerDeconvolution::process(const Photo &, int, int)
{
    throw 0;
}


static Magick::Image
normalizeImage(Magick::Image& image, int w, int h, bool center)
{
    int k_w = image.columns();
    int k_h = image.rows();
    Magick::Image nk(Magick::Geometry(w, h), Magick::Color(0,0,0));
    int o_x = (w-k_w)/2;
    int o_y = (h-k_h)/2;
    Magick::Pixels i_cache(image);
    nk.modifyImage();
    Magick::Pixels n_cache(nk);
#pragma omp parallel for
    for ( int y = 0 ; y < k_h ; ++y ) {
        const Magick::PixelPacket * k_pixel = i_cache.getConst(0, y, k_w, 1);
        Magick::PixelPacket * n_pixel;
        if (center)
            n_pixel = n_cache.get(o_x, o_y+y, k_w, 1);
        else
            n_pixel = n_cache.get(0, y, k_w, 1);
        for ( int x = 0 ; x < k_w ; ++x ) {
            n_pixel[x] = k_pixel[x];
        }
    }
    n_cache.sync();
    return nk;
}
static inline Magick::Image roll(Magick::Image& image, int o_x, int o_y)
{
    int w = image.columns();
    int h = image.rows();
    Magick::Image nk(Magick::Geometry(w, h), Magick::Color(0,0,0));
    Magick::Pixels i_cache(image);
    nk.modifyImage();
    Magick::Pixels n_cache(nk);
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        const Magick::PixelPacket * k_pixel = i_cache.getConst(0, y, w, 1);
        Magick::PixelPacket * n_pixel= n_cache.get(0, (y+o_y+h)%h, w, 1);
        for ( int x = 0 ; x < w ; ++x ) {
            n_pixel[(x+o_x+w)%w] = k_pixel[x];
        }
    }
    n_cache.sync();
    return nk;

}

static void deconv(Magick::Image& image, Magick::Image& kernel)
{
    std::list<Magick::Image> fft_image;
    std::list<Magick::Image> fft_kernel;
    Magick::Image nk = normalizeImage(kernel, qMax(image.columns(),image.rows()), qMax(image.columns(),image.rows()), true);
    Magick::Image ni = normalizeImage(image, qMax(image.columns(),image.rows()), qMax(image.columns(),image.rows()), false);

    Magick::Image nnk = roll(nk,-nk.columns()/2, -nk.rows()/2);

    Magick::forwardFourierTransformImage(&fft_image, ni, true);
    Magick::forwardFourierTransformImage(&fft_kernel, nnk, true);
    qDebug("fft_image.size = %ld", fft_image.size());
    qDebug("fft_kernel.size = %ld", fft_kernel.size());
    qDebug("w1=%ld, h1=%ld, w2=%ld, h2=%ld",
           fft_image.front().columns(),
           fft_image.front().rows(),
           fft_image.back().columns(),
           fft_image.back().rows()
           );
    /*
     * R = B / A
     * Rm = Bm / Am
     * Rp = mod( -Ap + Bp + 1.5, 1.0 );
     */
    Magick::Image& Bm=fft_image.front();
    Magick::Image& Bp=fft_image.back();
    Magick::Image& Am=fft_kernel.front();
    Magick::Image& Ap=fft_kernel.back();
    int w = Am.columns();
    int h = Am.rows();
    Magick::Image Rm(Magick::Geometry(w, h), Magick::Color(0,0,0));
    Magick::Image Rp(Magick::Geometry(w, h), Magick::Color(0,0,0));
    Rm.modifyImage();
    Rp.modifyImage();
    Magick::Pixels Am_cache(Am);
    Magick::Pixels Ap_cache(Ap);
    Magick::Pixels Bm_cache(Bm);
    Magick::Pixels Bp_cache(Bp);
    Magick::Pixels Rm_cache(Rm);
    Magick::Pixels Rp_cache(Rp);

    const Magick::PixelPacket *center_pxl = Am_cache.getConst(w/2, h/2, 1, 1);
    quantum_t red = center_pxl[0].red;
    quantum_t green = center_pxl[0].green;
    quantum_t blue = center_pxl[0].blue;

#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
       const Magick::PixelPacket *Am_pxl = Am_cache.getConst(0, y, w, 1);
       const Magick::PixelPacket *Ap_pxl = Ap_cache.getConst(0, y, w, 1);
       const Magick::PixelPacket *Bm_pxl = Bm_cache.getConst(0, y, w, 1);
       const Magick::PixelPacket *Bp_pxl = Bp_cache.getConst(0, y, w, 1);
       Magick::PixelPacket *Rm_pxl = Rm_cache.get(0, y, w, 1);
       Magick::PixelPacket *Rp_pxl = Rp_cache.get(0, y, w, 1);
       for ( int x = 0 ; x < w ; ++x ) {

           Q_UNUSED(Am_pxl);
           Q_UNUSED(Bm_pxl);

#define RM(comp) \
    Rm_pxl[x].comp = clamp( comp * (Bm_pxl[x].comp) / ( Am_pxl[x].comp?:1 )  )
           RM(red); RM(green); RM(blue);
#define mod(a,b) (a)%(b)
#define RP(comp) \
    Rp_pxl[x].comp = mod( - quantum_t(Ap_pxl[x].comp) + quantum_t(Bp_pxl[x].comp)+ 65536+32768, 65536)
           RP(red); RP(green); RP(blue);
       }
    }
    Rm_cache.sync();
    Rp_cache.sync();
    Rm.inverseFourierTransform(Rp, true);
    image=Rm;
}

void WorkerDeconvolution::play(QVector<QVector<Photo> > inputs, int n_outputs)
{
    Q_ASSERT( inputs.count() == 2 );

    if ( inputs[1].count() == 0 )
        return OperatorWorker::play(inputs, n_outputs);

    int k_count = inputs[1].count();
    m_inputs = inputs;
    play_prepareOutputs(n_outputs);
    int complete = qMin(1,k_count) * inputs[0].count();
    int n = 0;
    foreach(Photo photo, inputs[0]) {
        if (aborted())
            continue;
        Magick::Image& image = photo.image();
        Magick::Image& kernel = inputs[1][n%k_count].image();
        int w=image.columns();
        int h=image.rows();
        deconv(image, kernel);
        image.crop(Magick::Geometry(w, h));
        m_outputs[0].push_back(photo);
        emitProgress(n,complete, 1, 1);
        ++n;
    }
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
}
