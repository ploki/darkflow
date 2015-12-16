#include "opcmydecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"
#include "igamma.h"

class WorkerCMYDecompose : public OperatorWorker {
public:
    WorkerCMYDecompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        iGamma &labGammaReverse = iGamma::reverse_Lab();
        int p = 0,
                c = m_inputs[0].count();
        foreach(Photo pCyan, m_inputs[0]) {
            if (aborted())
                continue;
            Photo pMagenta(pCyan);
            Photo pYellow(pCyan);
            Photo pLuminance(pCyan);
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();

            try {
                Labize(pCyan.image(), pLuminance.image());
                emitProgress(p, c, 1, 4);

                labGammaReverse.applyOnImage(pLuminance.image());
                emitProgress(p, c, 2, 4);

                iCyan.modifyImage();
                iMagenta.modifyImage();
                iYellow.modifyImage();
                Magick::Pixels iCyan_cache(iCyan);
                Magick::Pixels iMagenta_cache(iMagenta);
                Magick::Pixels iYellow_cache(iYellow);
                int w = iCyan.columns();
                int h = iCyan.rows();
                int line = 0;
#pragma omp parallel for
                for ( int y = 0 ; y < h ; ++y ) {
                    Magick::PixelPacket *pxl_Cyan = iCyan_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Magenta = iMagenta_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Yellow = iYellow_cache.get(0, y, w, 1);
                    for ( int x = 0 ; x < w ; ++x ) {
                        pxl_Cyan[x].green = pxl_Cyan[x].blue = pxl_Cyan[x].red =
                                (quantum_t(pxl_Cyan[x].green) + quantum_t(pxl_Cyan[x].blue))/2;
                        pxl_Magenta[x].green = pxl_Magenta[x].blue = pxl_Magenta[x].red =
                                (quantum_t(pxl_Magenta[x].red) + quantum_t(pxl_Magenta[x].blue))/2;
                        pxl_Yellow[x].green = pxl_Yellow[x].blue = pxl_Yellow[x].red =
                                (quantum_t(pxl_Yellow[x].red) + quantum_t(pxl_Yellow[x].green))/2;
                    }
#pragma omp critical
                    {
                        emitProgress(p, c, h/2+line++/2, h);
                    }
                }
                iCyan_cache.sync();
                iMagenta_cache.sync();
                iYellow_cache.sync();
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
    Operator(OP_SECTION_COLOR, "LCMY Decompose", parent)
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
