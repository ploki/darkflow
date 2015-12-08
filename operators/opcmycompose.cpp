#include "opcmycompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"
#include "igamma.h"

class WorkerCMYCompose : public OperatorWorker {
public:
    WorkerCMYCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int lab_count = m_inputs[0].count();
        int photo_count = m_inputs[1].count();
        for (int i = 2 ; i < 4 ; ++i )
            if ( m_inputs[i].count() != photo_count ) {
                qWarning("Uneven photo count in LCMY Compose");
                emitFailure();
                return;
            }
        if ( lab_count && lab_count != photo_count ) {
            qWarning("Wrong number of Luminance photos in LCMY Compose");
            emitFailure();
            return;
        }
        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pCyan(m_inputs[1][i]);
            Photo pMagenta(m_inputs[2][i]);
            Photo pYellow(m_inputs[3][i]);
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();
            iCyan.modifyImage();
            Magick::Pixels iCyan_cache(iCyan);
            Magick::Pixels iMagenta_cache(iMagenta);
            Magick::Pixels iYellow_cache(iYellow);
            int w = iCyan.columns();
            int h = iCyan.rows();
#pragma omp parallel for
            for ( int y = 0 ; y < h ; ++y ) {
                Magick::PixelPacket *pxl_Cyan = iCyan_cache.get(0, y, w, 1);
                const Magick::PixelPacket *pxl_Magenta = iMagenta_cache.getConst(0, y, w, 1);
                const Magick::PixelPacket *pxl_Yellow = iYellow_cache.getConst(0, y, w, 1);
                for ( int x = 0 ; x < w ; ++x ) {
                    quantum_t rgb[3];
                    rgb[0] =  - pxl_Cyan[x].red + pxl_Magenta[x].green + pxl_Yellow[x].blue;
                    rgb[1] =    pxl_Cyan[x].red - pxl_Magenta[x].green + pxl_Yellow[x].blue;
                    rgb[2] =    pxl_Cyan[x].red + pxl_Magenta[x].green - pxl_Yellow[x].blue;
                    pxl_Cyan[x].red = clamp(rgb[0]);
                    pxl_Cyan[x].green = clamp(rgb[1]);
                    pxl_Cyan[x].blue = clamp(rgb[2]);
                }
            }
            iCyan_cache.sync();
            if ( lab_count ) {
                Photo lPhoto(m_inputs[0][i]);
                iGamma& labGamma = iGamma::Lab();
                labGamma.applyOn(lPhoto);
                unLabize(pCyan.image(), lPhoto.image());
            }
            outputPush(0, pCyan);
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
