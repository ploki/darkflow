#include "oprgbcompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "algorithm.h"
#include "console.h"

static Photo
blackDot()
{
    return Photo(Magick::Image(Magick::Geometry(1,1), Magick::Color(0,0,0)), Photo::Linear);
}

class WorkerRGBCompose : public OperatorWorker {
public:
    WorkerRGBCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int l_count = m_inputs[0].count();
        int r_count = m_inputs[1].count();
        int g_count = m_inputs[2].count();
        int b_count = m_inputs[3].count();
        int photo_count = qMax(qMax(qMax(l_count, r_count), g_count), b_count);

        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pLuminance;
            Photo pRed;
            Photo pGreen;
            Photo pBlue;
            if ( l_count )
                pLuminance = m_inputs[0][i%l_count];
            else
                pLuminance = blackDot();

            if ( r_count )
                pRed = m_inputs[1][i%r_count];
            else
                pRed = blackDot();

            if ( g_count )
                pGreen = m_inputs[2][i%g_count];
            else
                pGreen = blackDot();

            if ( b_count )
                pBlue = m_inputs[3][i%b_count];
            else
                pBlue = blackDot();

            try {
                Magick::Image& iLuminance = pLuminance.image();
                Magick::Image& iRed = pRed.image();
                Magick::Image& iGreen = pGreen.image();
                Magick::Image& iBlue = pBlue.image();

                unsigned w = qMax(qMax(qMax(iLuminance.columns(),iRed.columns()), iGreen.columns()), iBlue.columns());
                unsigned h = qMax(qMax(qMax(iLuminance.rows(),iRed.rows()), iGreen.rows()), iBlue.rows());

                if ( iLuminance.columns() != w || iLuminance.rows() != h )
                    iLuminance.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
                if ( iRed.columns() != w || iRed.rows() != h )
                    iRed.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
                if ( iGreen.columns() != w || iGreen.rows() != h )
                    iGreen.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
                if ( iBlue.columns() != w || iBlue.rows() != h )
                    iBlue.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);

                Magick::Pixels iRed_cache(iRed);
                Magick::Pixels iGreen_cache(iGreen);
                Magick::Pixels iBlue_cache(iBlue);
                Magick::Pixels iLuminance_cache(iLuminance);

                Photo photo(Photo::Linear);
                photo.createImage(w, h);
                photo.setSequenceNumber(i);
                photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
                photo.setTag(TAG_NAME, "LRGB Composition");
                Magick::Pixels iPhoto_cache(photo.image());
                Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
                int line = 0;
#pragma omp parallel for dfl_threads(4, iLuminance, iRed, iGreen, iBlue)
                for ( int y = 0 ; y < int(h) ; ++y ) {
                    const Magick::PixelPacket *pxl_Red = iRed_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Green = iGreen_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Blue = iBlue_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Luminance = iLuminance_cache.getConst(0, y, w, 1);
                    if ( m_error || !pxl_Red || !pxl_Green || !pxl_Blue || !pxl_Luminance ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( unsigned x = 0 ; x < w ; ++x ) {
                        quantum_t red = pxl_Red?pxl_Red[x].red:0;
                        quantum_t green = pxl_Green?pxl_Green[x].green:0;
                        quantum_t blue = pxl_Blue?pxl_Blue[x].blue:0;

                        if ( l_count ) {
                            double lum = .2126L*(pxl_Luminance?pxl_Luminance[x].red:0) +
                                    .7152L*(pxl_Luminance?pxl_Luminance[x].green:0) +
                                    .0722L*(pxl_Luminance?pxl_Luminance[x].blue:0);
                            double cur = .2126L*red +
                                    .7152L*green +
                                    .0722L*blue;
                            double mul = lum/cur;
                            red = clamp<quantum_t>(DF_ROUND(mul*red));
                            green =  clamp<quantum_t>(DF_ROUND(mul*green));
                            blue = clamp<quantum_t>(DF_ROUND(mul*blue));
                        }
                        pxl[y*w+x].red=red;
                        pxl[y*w+x].green=green;
                        pxl[y*w+x].blue=blue;
                    }
#pragma omp critical
                    {
                        emitProgress(i, photo_count,line++, h);
                    }
                }
                iPhoto_cache.sync();
                outputPush(0, photo);
                emitProgress(i, photo_count, 1, 1);
            }
            catch (std::exception &e) {
                if (l_count)
                    setError(pLuminance, e.what());
                if (r_count)
                    setError(pRed, e.what());
                if (g_count)
                    setError(pGreen, e.what());
                if (b_count)
                    setError(pBlue, e.what());
                emitFailure();
                return;
            }
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }
};

OpRGBCompose::OpRGBCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, "LRGB Compose", Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Luminance", "Luminance", OperatorInput::Set, this));
    addInput(new OperatorInput("Red", "Red", OperatorInput::Set, this));
    addInput(new OperatorInput("Green", "Green", OperatorInput::Set, this));
    addInput(new OperatorInput("Blue", "Blue", OperatorInput::Set, this));
    addOutput(new OperatorOutput("RGB", "RGB", this));
}

OpRGBCompose *OpRGBCompose::newInstance()
{
    return new OpRGBCompose(m_process);
}

OperatorWorker *OpRGBCompose::newWorker()
{
    return new WorkerRGBCompose(m_thread, this);
}
