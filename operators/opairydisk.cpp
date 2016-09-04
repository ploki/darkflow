#include "opairydisk.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "photo.h"
#include "console.h"
#include "algorithm.h"
#include "operatorparameterslider.h"

using Magick::Quantum;
#define lred  612e-9
#define lgreen  549e-9
#define lblue  450e-9

static const double lambda[] = { lred, lgreen, lblue };

class AiryDisk : public OperatorWorker
{
    double m_diam;
    double m_f;
    double m_pixel_sz;
    double m_offset;
    int m_w;
    int m_precision;
    double m_I0;
public:
    AiryDisk(double diam,
             double f,
             double pixel_sz,
             double offset,
             double w,
             double precision,
             QThread *thread, OpAiryDisk *op) :
        OperatorWorker(thread, op),
        m_diam(diam),
        m_f(f),
        m_pixel_sz(pixel_sz),
        m_offset(offset),
        m_w(w),
        m_precision(precision),
        m_I0(QuantumRange)
    {
    }
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play() {
        Photo photo(Photo::Linear);
        photo.setIdentity(m_operator->uuid());
        photo.createImage(m_w, m_w);
        if (photo.isComplete()) {
            Magick::Image& image = photo.image();
            std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(image));
            int w = image.columns();
            int h = image.rows();
            dfl_parallel_for(y, 0, h, 4, (image), {
                if (aborted())
                    continue;
                emit progress(y, h);
                Magick::PixelPacket *pixels = cache->get(0,y,w,1);
                if ( m_error || !pixels ) {
                    if ( !m_error )
                        dflError(DF_NULL_PIXELS);
                    m_error = true;
                    continue;
                }
                for (int x = 0 ; x < w ; ++x ) {
                    double rgb[3] = {0, 0, 0};
                    for ( int jy = 0 ; jy < m_precision ; ++jy ) {
                        for ( int jx = 0 ; jx < m_precision ; ++jx ) {
                            double r = m_pixel_sz *
                                    sqrt(pow(double(x+double(jx)/m_precision+m_offset)-double(w)/2.,2) +
                                         pow(double(y+double(jy)/m_precision+m_offset)-double(h)/2., 2));
                            for ( int c = 0 ; c < 3 ; ++c ) {
                                double sin_theta = r/m_f;
                                double xx = m_diam * sin_theta / lambda[c];
                                double res = QuantumRange;
                                if ( xx != 0 )
                                    res = m_I0 * pow(2*j1(M_PI*xx)/(M_PI*xx),2);
                                rgb[c] += res;
                            }
                        }
                    }
                    int sq_p = m_precision*m_precision;
                    pixels[x].red = clamp<quantum_t>(rgb[0] / (sq_p));
                    pixels[x].green = clamp<quantum_t>(rgb[1] / (sq_p));
                    pixels[x].blue = clamp<quantum_t>(rgb[2] / (sq_p));
                }
                cache->sync();
            });
            if (aborted()) {
                emitFailure();
            }
            else {
                photo.setTag(TAG_NAME, "Airy Disk");
                outputPush(0, photo);
                emitSuccess();
            }
        }
    }
};

OpAiryDisk::OpAiryDisk(Process *parent) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Airy Disk"), Operator::NA, parent),
    m_diameter(new OperatorParameterSlider("diameter",tr("Diameter"), tr("Airy disk - Diameter in mm"), Slider::Value, Slider::Logarithmic, Slider::Real, 20, 1000, 200, 1, 10000, Slider::FilterPixels, this)),
    m_focal(new OperatorParameterSlider("focal",tr("Focal length"), tr("Airy disk - Focal length in mm"), Slider::Value, Slider::Logarithmic, Slider::Real, 200, 5000, 2000, 1, 100000, Slider::FilterPixels, this)),
    m_pixel_sz(new OperatorParameterSlider("pixelSize",tr("Pixel size"), tr("Airy disk - Pixel size in µm"), Slider::Value, Slider::Logarithmic, Slider::Real, 1, 32, 8.45, 0.001, 100, Slider::FilterPixels, this)),
    m_offset(new OperatorParameterSlider("offset",tr("Offset"), tr("Airy disk - Offset in fraction of pixels"), Slider::Value, Slider::Linear, Slider::Real, 0, 1, 0.5, 0, 1, Slider::FilterPixels, this)),
    m_width(new OperatorParameterSlider("width",tr("Width"), tr("Airy disk - Image width"), Slider::Value, Slider::Linear, Slider::Integer, 0, 100, 50, 0, 2000, Slider::FilterPixels, this)),
    m_precision(new OperatorParameterSlider("precision",tr("Precision"), tr("Airy disk - Precision in subdivisions"), Slider::Value, Slider::Linear, Slider::Integer, 1, 10, 5, 1, 200, Slider::FilterPixels, this))
{
    addOutput(new OperatorOutput(tr("Airy Disk"), this));
    addParameter(m_diameter);
    addParameter(m_focal);
    addParameter(m_pixel_sz);
    addParameter(m_offset);
    addParameter(m_width);
    addParameter(m_precision);
}

OpAiryDisk::~OpAiryDisk()
{

}

OpAiryDisk *OpAiryDisk::newInstance()
{
    return new OpAiryDisk(m_process);
}

OperatorWorker *OpAiryDisk::newWorker()
{
    return new AiryDisk(m_diameter->value()/1000.,
                        m_focal->value()/1000.,
                        m_pixel_sz->value()/1000000.,
                        m_offset->value(),
                        m_width->value(),
                        m_precision->value(),
                        m_thread, this);
}
