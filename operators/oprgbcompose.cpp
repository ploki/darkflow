#include "oprgbcompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"
#include "igamma.h"

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

                Photo photo(Photo::Linear);
                photo.createImage(w, h);
                photo.setSequenceNumber(i);
                photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
                photo.setTag("Name", "LRGB Composition");
                Magick::Pixels iPhoto_cache(photo.image());
                Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
                int line = 0;
#pragma omp parallel for
                for ( unsigned y = 0 ; y < h ; ++y ) {
                    const Magick::PixelPacket *pxl_Red = iRed_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Green = iGreen_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Blue = iBlue_cache.getConst(0, y, w, 1);
                    for ( unsigned x = 0 ; x < w ; ++x ) {
                        pxl[y*w+x].red = pxl_Red?pxl_Red[x].red:0;
                        pxl[y*w+x].green = pxl_Green?pxl_Green[x].green:0;
                        pxl[y*w+x].blue = pxl_Blue?pxl_Blue[x].blue:0;
                    }
#pragma omp critical
                    {
                        emitProgress(i, photo_count,(line++)/2, h);
                    }
                }
                iPhoto_cache.sync();
                if ( l_count ) {
                    iGamma& labGamma = iGamma::Lab();
                    labGamma.applyOn(pLuminance);
                    emitProgress(i, photo_count, 3, 4);
                    unLabize(photo.image(), pLuminance.image());
                }
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
    Operator(OP_SECTION_COLOR, "LRGB Compose", parent)
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
