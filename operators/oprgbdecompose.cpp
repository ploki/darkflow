#include "oprgbdecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"

class WorkerRGBDecompose : public OperatorWorker {
public:
    WorkerRGBDecompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int p = 0,
                c = m_inputs[0].count();
        foreach(Photo pRed, m_inputs[0]) {
            if (aborted())
                continue;
            Photo pGreen(pRed);
            Photo pBlue(pRed);
            Photo pLuminance(pRed);
            Magick::Image& iRed = pRed.image();
            Magick::Image& iGreen = pGreen.image();
            Magick::Image& iBlue = pBlue.image();
            Magick::Image& iLuminance = pLuminance.image();

            try {
                iRed.modifyImage();
                iGreen.modifyImage();
                iBlue.modifyImage();
                iLuminance.modifyImage();
                Magick::Pixels iRed_cache(iRed);
                Magick::Pixels iGreen_cache(iGreen);
                Magick::Pixels iBlue_cache(iBlue);
                Magick::Pixels iLuminance_cache(iLuminance);
                int w = iRed.columns();
                int h = iRed.rows();
                int line = 0;
#pragma omp parallel for
                for ( int y = 0 ; y < h ; ++y ) {
                    Magick::PixelPacket *pxl_Red = iRed_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Green = iGreen_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Blue = iBlue_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Luminance = iLuminance_cache.get(0, y, w, 1);
                    for ( int x = 0 ; x < w ; ++x ) {
                        pxl_Red[x].green = pxl_Red[x].blue = pxl_Red[x].red;
                        pxl_Green[x].red = pxl_Green[x].blue = pxl_Green[x].green;
                        pxl_Blue[x].red = pxl_Blue[x].green = pxl_Blue[x].blue;
                        pxl_Luminance[x].red =
                        pxl_Luminance[x].green =
                        pxl_Luminance[x].blue =
                                round(.2126L * pxl_Red[x].red +
                                      .7152L * pxl_Green[x].green +
                                      .0722L * pxl_Blue[x].blue);
                    }
#pragma omp critical
                    {
                        emitProgress(p, c, line++, h);
                    }
                }
                iRed_cache.sync();
                iGreen_cache.sync();
                iBlue_cache.sync();
                iLuminance_cache.sync();
                outputPush(0, pLuminance);
                outputPush(1, pRed);
                outputPush(2, pGreen);
                outputPush(3, pBlue);
                emitProgress(p, c, 1, 1);
                ++p;
            }
            catch(std::exception &e) {
                setError(pRed, e.what());
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

OpRGBDecompose::OpRGBDecompose(Process *parent) :
    Operator(OP_SECTION_COLOR, "LRGB Decompose", Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Images set", "Images set", OperatorInput::Set, this));
    addOutput(new OperatorOutput("Luminance", "Luminance", this));
    addOutput(new OperatorOutput("Red", "Red", this));
    addOutput(new OperatorOutput("Green", "Green", this));
    addOutput(new OperatorOutput("Blue", "Blue", this));
}

OpRGBDecompose *OpRGBDecompose::newInstance()
{
    return new OpRGBDecompose(m_process);
}

OperatorWorker *OpRGBDecompose::newWorker()
{
    return new WorkerRGBDecompose(m_thread, this);
}
