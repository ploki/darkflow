#include "oplevelpercentile.h"

#include "ports.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"

using Magick::Quantum;

class WorkerLevelPercentile : public OperatorWorker {
public:
    WorkerLevelPercentile(qreal blackPoint, qreal whitePoint, qreal gamma, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_blackPoint(blackPoint),
        m_whitePoint(whitePoint),
        m_gamma(gamma)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);

        Magick::Image& image = newPhoto.image();
        int h = image.rows();
        int w = image.columns();
        Q_ASSERT(QuantumRange + 1 == 1<<16);
        unsigned int histo[QuantumRange+1][3] = {};
        Magick::Pixels pixel_cache(image);
        const Magick::PixelPacket *pixels = pixel_cache.getConst(0,0, w, h);
#pragma omp parallel for dfl_threads(4)
        for ( int y = 0 ; y < h ; ++y ) {
                if ( !pixels ) continue;
                for ( int x = 0 ; x < w ; ++x ) {
                        quantum_t r=pixels[x].red;
                        quantum_t g=pixels[x].green;
                        quantum_t b=pixels[x].blue;
                        atomic_incr(&histo[r][0]);
                        atomic_incr(&histo[g][1]);
                        atomic_incr(&histo[b][2]);
                }
        }

        quantum_t whitepoint=0,blackpoint=0;
        double perc_wp=(1.-m_whitePoint)*3*w*h;
        double perc_bp=(m_blackPoint)*3*w*h;

        int total=0;
        //white point
        for ( int i=int(QuantumRange) ; i>=0 ; --i ) {
                total+=histo[i][0]+histo[i][1]+histo[i][2];
                if ( total >= perc_wp ) {
                        whitepoint=i;
                        break;
                }
        }

        total=0;
        //black point
        for ( int i=0 ; i <= int(QuantumRange) ; ++i ) {
                total+=histo[i][0]+histo[i][1]+histo[i][2];
                if ( total >= perc_bp ) {
                        blackpoint=i;
                        break;
                }
        }
        dflDebug("bp=%d, wp=%d, perc_bp=%f, perc_wp=%f", blackpoint, whitepoint, perc_bp, perc_wp);

        newPhoto.image().level(blackpoint,
                               whitepoint,
                               m_gamma);
        newPhoto.curve().level(blackpoint,
                               whitepoint,
                               m_gamma);
        return newPhoto;
    }

private:
    qreal m_blackPoint;
    qreal m_whitePoint;
    qreal m_gamma;
};

OpLevelPercentile::OpLevelPercentile(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "Level Percentile"), Operator::NonHDR, parent),
    m_blackPoint(new OperatorParameterSlider("blackPoint", tr("Black Point"), tr("Level Black Point"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 0.05, 0, 1, Slider::FilterPercent, this)),
    m_whitePoint(new OperatorParameterSlider("blackPoint", tr("White Point"), tr("Level White Point"), Slider::Percent, Slider::Linear, Slider::Real, 0, 1, 0.95, 0, 1, Slider::FilterPercent, this)),
    m_gamma(new OperatorParameterSlider("gamma", tr("Gamma"), tr("Level Gamma"), Slider::Value, Slider::Logarithmic, Slider::Real, 0.1, 10, 1, 0.01, 10, Slider::FilterNothing, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    addParameter(m_blackPoint);
    addParameter(m_whitePoint);
    addParameter(m_gamma);
}

OpLevelPercentile *OpLevelPercentile::newInstance()
{
    return new OpLevelPercentile(m_process);
}

OperatorWorker *OpLevelPercentile::newWorker()
{
return new WorkerLevelPercentile(m_blackPoint->value(),
                                 m_whitePoint->value(),
                                 m_gamma->value(),
                                 m_thread, this);
}
