#include "opcmydecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "console.h"

class WorkerCMYDecompose : public OperatorWorker {
public:
    WorkerCMYDecompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int p = 0,
                c = m_inputs[0].count();
        foreach(Photo photo, m_inputs[0]) {
            if (aborted())
                continue;
            Photo pCyan(photo);
            Photo pMagenta(photo);
            Photo pYellow(photo);
            Photo pLuminance(photo);
            Magick::Image& srcImage = photo.image();
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();
            Magick::Image& iLuminance = pLuminance.image();

            try {
                ResetImage(iCyan);
                ResetImage(iMagenta);
                ResetImage(iYellow);
                ResetImage(iLuminance);
                Magick::Pixels src_cache(srcImage);
                Magick::Pixels iCyan_cache(iCyan);
                Magick::Pixels iMagenta_cache(iMagenta);
                Magick::Pixels iYellow_cache(iYellow);
                Magick::Pixels iLuminance_cache(iLuminance);
                int w = srcImage.columns();
                int h = srcImage.rows();
                int line = 0;
#pragma omp parallel for dfl_threads(4, srcImage, iCyan, iMagenta, iYellow, iLuminance)
                for ( int y = 0 ; y < h ; ++y ) {
                    const Magick::PixelPacket *src = src_cache.getConst(0, y, w, 1);
                    Magick::PixelPacket *pxl_Cyan = iCyan_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Magenta = iMagenta_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Yellow = iYellow_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Luminance = iLuminance_cache.get(0, y, w, 1);
                    if ( m_error || !src || !pxl_Cyan || !pxl_Magenta || !pxl_Yellow || !pxl_Luminance ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( int x = 0 ; x < w ; ++x ) {
                        pxl_Luminance[x].red =
                                pxl_Luminance[x].green =
                                pxl_Luminance[x].blue =
                                        DF_ROUND(.2126L * src[x].red +
                                                 .7152L * src[x].green +
                                                 .0722L * src[x].blue);
                        pxl_Cyan[x].green = pxl_Cyan[x].blue = pxl_Cyan[x].red =
                                (quantum_t(src[x].green) + quantum_t(src[x].blue))/2;
                        pxl_Magenta[x].green = pxl_Magenta[x].blue = pxl_Magenta[x].red =
                                (quantum_t(src[x].red) + quantum_t(src[x].blue))/2;
                        pxl_Yellow[x].green = pxl_Yellow[x].blue = pxl_Yellow[x].red =
                                (quantum_t(src[x].red) + quantum_t(src[x].green))/2;
                    }
                    iCyan_cache.sync();
                    iMagenta_cache.sync();
                    iYellow_cache.sync();
                    iLuminance_cache.sync();
#pragma omp critical
                    {
                        emitProgress(p, c, line++, h);
                    }
                }
                outputPush(0, pLuminance);
                outputPush(1, pCyan);
                outputPush(2, pMagenta);
                outputPush(3, pYellow);
                emitProgress(p, c, 1, 1);
                ++p;
            }
            catch(std::exception &e) {
                setError(pCyan, e.what());
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

OpCMYDecompose::OpCMYDecompose(Process *parent) :
    Operator(OP_SECTION_COLOR, "LCMY Decompose", Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Images set", "Images set", OperatorInput::Set, this));
    addOutput(new OperatorOutput("Luminance", "Luminance", this));
    addOutput(new OperatorOutput("Cyan", "Cyan", this));
    addOutput(new OperatorOutput("Magenta", "Magenta", this));
    addOutput(new OperatorOutput("Yellow", "Yellow", this));

}

OpCMYDecompose *OpCMYDecompose::newInstance()
{
    return new OpCMYDecompose(m_process);
}

OperatorWorker *OpCMYDecompose::newWorker()
{
    return new WorkerCMYDecompose(m_thread, this);
}
