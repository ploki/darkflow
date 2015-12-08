#include "opcmycompose.h"
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

class WorkerCMYCompose : public OperatorWorker {
public:
    WorkerCMYCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int l_count = m_inputs[0].count();
        int c_count = m_inputs[1].count();
        int m_count = m_inputs[2].count();
        int y_count = m_inputs[3].count();
        int photo_count = qMax(qMax(qMax(l_count, c_count), m_count), y_count);

        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pLuminance;
            Photo pCyan;
            Photo pMagenta;
            Photo pYellow;
            if ( l_count )
                pLuminance = m_inputs[0][i%l_count];
            else
                pLuminance = blackDot();

            if ( c_count )
                pCyan = m_inputs[1][i%c_count];
            else
                pCyan = blackDot();

            if ( m_count )
                pMagenta = m_inputs[2][i%m_count];
            else
                pMagenta = blackDot();

            if ( y_count )
                pYellow = m_inputs[3][i%y_count];
            else
                pYellow = blackDot();

            Magick::Image& iLuminance = pLuminance.image();
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();

            int w = qMax(qMax(qMax(iLuminance.columns(),iCyan.columns()), iMagenta.columns()), iYellow.columns());
            int h = qMax(qMax(qMax(iLuminance.rows(),iCyan.rows()), iMagenta.rows()), iYellow.rows());
            iLuminance.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            iCyan.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            iMagenta.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            iYellow.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);

            Magick::Pixels iCyan_cache(iCyan);
            Magick::Pixels iMagenta_cache(iMagenta);
            Magick::Pixels iYellow_cache(iYellow);


            Photo photo(Photo::Linear);
            photo.createImage(w, h);
            photo.setSequenceNumber(i);
            photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
            photo.setTag("Name", "LCMY Composition");
            Magick::Pixels iPhoto_cache(photo.image());
            Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
#pragma omp parallel for
            for ( int y = 0 ; y < h ; ++y ) {
                const Magick::PixelPacket *pxl_Cyan = iCyan_cache.getConst(0, y, w, 1);
                const Magick::PixelPacket *pxl_Magenta = iMagenta_cache.getConst(0, y, w, 1);
                const Magick::PixelPacket *pxl_Yellow = iYellow_cache.getConst(0, y, w, 1);
                for ( int x = 0 ; x < w ; ++x ) {
                    quantum_t rgb[3];
                    rgb[0] =  - (pxl_Cyan?pxl_Cyan[x].red:0) + (pxl_Magenta?pxl_Magenta[x].green:0) + (pxl_Yellow?pxl_Yellow[x].blue:0);
                    rgb[1] =    (pxl_Cyan?pxl_Cyan[x].red:0) - (pxl_Magenta?pxl_Magenta[x].green:0) + (pxl_Yellow?pxl_Yellow[x].blue:0);
                    rgb[2] =    (pxl_Cyan?pxl_Cyan[x].red:0) + (pxl_Magenta?pxl_Magenta[x].green:0) - (pxl_Yellow?pxl_Yellow[x].blue:0);
                    pxl[y*w+x].red = clamp(rgb[0]);
                    pxl[y*w+x].green = clamp(rgb[1]);
                    pxl[y*w+x].blue = clamp(rgb[2]);
                }
            }
            iPhoto_cache.sync();
            if ( l_count ) {
                iGamma& labGamma = iGamma::Lab();
                labGamma.applyOn(pLuminance);
                unLabize(photo.image(), pLuminance.image());
            }
            outputPush(0, photo);
            emitProgress(i, photo_count, 1, 1);
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }
};

OpCMYCompose::OpCMYCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, "LCMY Compose", parent)
{
    addInput(new OperatorInput("Luminance", "Luminance", OperatorInput::Set, this));
    addInput(new OperatorInput("Cyan", "Cyan", OperatorInput::Set, this));
    addInput(new OperatorInput("Magenta", "Magenta", OperatorInput::Set, this));
    addInput(new OperatorInput("Yellow", "Yellow", OperatorInput::Set, this));
    addOutput(new OperatorOutput("RGB", "RGB", this));
}

OpCMYCompose *OpCMYCompose::newInstance()
{
    return new OpCMYCompose(m_process);
}

OperatorWorker *OpCMYCompose::newWorker()
{
    return new WorkerCMYCompose(m_thread, this);
}
