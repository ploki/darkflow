#include "selectivelabfilter.h"
#include "hdr.h"
#include "cielab.h"
#include "ports.h"

SelectiveLabFilter::SelectiveLabFilter(int hue,
                                       int coverage,
                                       qreal saturation,
                                       bool strict,
                                       qreal exposure,
                                       bool insideSelection,
                                       bool exposureStrict,
                                       QObject *parent) :
    Algorithm(false,parent),
    m_hue(hue),
    m_coverage(coverage),
    m_saturation(saturation),
    m_strict(strict),
    m_exposure(exposure),
    m_insideSelection(insideSelection),
    m_exposureStrict(exposureStrict)

{
}

void SelectiveLabFilter::applyOnImage(Magick::Image &image, bool hdr)
{
    int     h = image.rows(),
            w = image.columns();
    Magick::Image srcImage(image);
    ResetImage(image);
    Magick::Pixels src_cache(srcImage);
    Magick::Pixels pixel_cache(image);

    bool inv_sat=false;
    if ( m_coverage < 0 ) {
        m_coverage = -m_coverage;
        inv_sat = true;
    }

    //calcul de la puissance de largeur
    double theta = M_PI - M_PI * (double(m_coverage)/2) / 180.;
    double puissance = 0;
    if ( m_coverage != 0 ) {
        puissance = (log(-M_LN2/(log(-(cos(theta)-1.)/2.)))+2.*M_LN2)/(M_LN2);
        puissance = pow(2.,puissance);
    }
    //calcul de l'angle d'application
    theta = M_PI * double((360+m_hue)%360)/180.;


    double saturation = m_saturation;
    double value = m_exposure;
    double bias_sat = m_strict?0:1;
    double bias_val = m_exposureStrict?0:1;
    bool inv_val = !m_insideSelection;

#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        const Magick::PixelPacket *src = src_cache.getConst(0,y,w,1);
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        if ( !pixels ) continue;

        for ( int x = 0 ; x < w ; ++x ) {

            double rgb[3];
            if (hdr) {
                rgb[0] = fromHDR(src[x].red);
                rgb[1] = fromHDR(src[x].green);
                rgb[2] = fromHDR(src[x].blue);
            }
            else {
                rgb[0] = src[x].red;
                rgb[1] = src[x].green;
                rgb[2] = src[x].blue;
            }

            double lab[3];

            RGB_to_LinearLab(rgb,lab);

            double module = sqrt(lab[1]*lab[1]+lab[2]*lab[2]);

            double arg;
            if ( lab[2] > 0 )
                arg = M_PI/2-atan(-lab[1]/lab[2]);
            else
                arg = -M_PI/2-atan(-lab[1]/lab[2]);

            double mul= pow((1.-cos(arg+theta))/2.,puissance);

            // ne pas augmenter la luminosité des parties non saturées d'où le *module/80.8284
            // (il semble que 100 soit la valeur max du module)
            double mul_val= inv_val?1.-mul:mul;
            lab[0]*=(bias_val+(value-bias_val)*mul_val*module/80.8284);

            double mul_sat = inv_sat?1.-mul:mul;
            module*=(bias_sat+(saturation-bias_sat)*mul_sat);

            lab[1]=-module * cos(arg);
            lab[2]=module * sin(arg);
            LinearLab_to_RGB(lab,rgb);
            if ( hdr) {
                pixels[x].red=toHDR(rgb[0]);
                pixels[x].green=toHDR(rgb[1]);
                pixels[x].blue=toHDR(rgb[2]);
            }
            else {
                pixels[x].red=rgb[0];
                pixels[x].green=rgb[1];
                pixels[x].blue=rgb[2];
            }
        }
    }
#pragma omp barrier
    pixel_cache.sync();

}
