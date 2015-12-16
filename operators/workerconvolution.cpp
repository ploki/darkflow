#include "opconvolution.h"
#include "workerconvolution.h"
#include <list>
#include "algorithm.h"

using Magick::Quantum;

WorkerConvolution::WorkerConvolution(qreal luminosity, QThread *thread, OpConvolution *op) :
    OperatorWorker(thread, op),
    m_luminosity(luminosity)
{
}

Photo WorkerConvolution::process(const Photo &photo, int, int)
{
    return photo;
}


static inline Magick::Image
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

void WorkerConvolution::conv(Magick::Image& image, Magick::Image& kernel, qreal luminosity)
{
#ifdef USING_GRAPHICSMAGICK
    Q_UNUSED(image);
    Q_UNUSED(kernel);
    Q_UNUSED(luminosity);
    dflCritical("Fourier Transformation not available with GraphicsMagick");
    return;
#else
    std::list<Magick::Image> fft_image;
    std::list<Magick::Image> fft_kernel;
    Magick::Image nk = normalizeImage(kernel, qMax(image.columns(),image.rows()), qMax(image.columns(),image.rows()), true);
    Magick::Image ni = normalizeImage(image, qMax(image.columns(),image.rows()), qMax(image.columns(),image.rows()), false);

    Magick::Image nnk = roll(nk,-nk.columns()/2, -nk.rows()/2);

    Magick::forwardFourierTransformImage(&fft_image, ni, true);
    Magick::forwardFourierTransformImage(&fft_kernel, nnk, true);
    dflDebug("fft_image.size = %ld", fft_image.size());
    dflDebug("fft_kernel.size = %ld", fft_kernel.size());
    dflDebug("w1=%ld, h1=%ld, w2=%ld, h2=%ld",
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

    const Magick::PixelPacket *center_pxl = Bm_cache.getConst(w/2, h/2, 1, 1);
    quantum_t red = center_pxl[0].red;
    quantum_t green = center_pxl[0].green;
    quantum_t blue = center_pxl[0].blue;
    Q_UNUSED(red); Q_UNUSED(green); Q_UNUSED(blue);
    dflDebug("comp.red=%d, luminosity=%f",red, 1./luminosity);
    dflDebug("comp.green=%d, luminosity=%f",green, 1./luminosity);
    dflDebug("comp.blue=%d, luminosity=%f",blue, 1./luminosity);

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
           Q_UNUSED(luminosity);
#define RM(comp) \
    Rm_pxl[x].comp = clamp( luminosity*double(Bm_pxl[x].comp)*double(Am_pxl[x].comp))
           RM(red); RM(green); RM(blue);
#define mod(a,b) (a)%(b)
#define RP(comp) \
    Rp_pxl[x].comp = mod( quantum_t(Bp_pxl[x].comp) + quantum_t(Ap_pxl[x].comp) + 32768, 65536)
           RP(red); RP(green); RP(blue);
       }
    }
    Rm_cache.sync();
    Rp_cache.sync();
    Rm.inverseFourierTransform(Rp, true);
    image=Rm;
#endif
}

void WorkerConvolution::play()
{
    Q_ASSERT( m_inputs.count() == 2 );

    if ( m_inputs[1].count() == 0 )
        return OperatorWorker::play();

    int k_count = m_inputs[1].count();
    int complete = qMin(1,k_count) * m_inputs[0].count();
    int n = 0;
    foreach(Photo photo, m_inputs[0]) {
        if (aborted())
            continue;
        try {
            Magick::Image& image = photo.image();
            Magick::Image& kernel = m_inputs[1][n%k_count].image();
            int w=image.columns();
            int h=image.rows();
            conv(image, kernel, m_luminosity);
            image.page(Magick::Geometry(0,0,0,0));
            image.crop(Magick::Geometry(w, h));
            outputPush(0, photo);
            emitProgress(n,complete, 1, 1);
            ++n;
        }
        catch (std::exception &e) {
            setError(photo, e.what());
            setError(m_inputs[1][n%k_count], e.what());
        }
    }
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
}
