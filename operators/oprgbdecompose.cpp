#include "oprgbdecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"
#include "igamma.h"

class WorkerRGBDecompose : public OperatorWorker {
public:
    WorkerRGBDecompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play(QVector<QVector<Photo> > inputs, int n_outputs) {
        iGamma &labGammaReverse = iGamma::reverse_Lab();
        m_inputs = inputs;
        play_prepareOutputs(n_outputs);
        int p = 0,
                c = inputs[0].count();
        foreach(Photo pRed, inputs[0]) {
            if (aborted())
                continue;
            Photo pGreen(pRed);
            Photo pBlue(pRed);
            Photo pLuminance(pRed);
            Magick::Image& iRed = pRed.image();
            Magick::Image& iGreen = pGreen.image();
            Magick::Image& iBlue = pBlue.image();

            Labize(pRed.image(), pLuminance.image());
            labGammaReverse.applyOnImage(pLuminance.image());
            iRed.modifyImage();
            iGreen.modifyImage();
            iBlue.modifyImage();
            Magick::Pixels iRed_cache(iRed);
            Magick::Pixels iGreen_cache(iGreen);
            Magick::Pixels iBlue_cache(iBlue);
            int w = iRed.columns();
            int h = iRed.rows();
#pragma omp parallel for
            for ( int y = 0 ; y < h ; ++y ) {
                Magick::PixelPacket *pxl_Red = iRed_cache.get(0, y, w, 1);
                Magick::PixelPacket *pxl_Green = iGreen_cache.get(0, y, w, 1);
                Magick::PixelPacket *pxl_Blue = iBlue_cache.get(0, y, w, 1);
                for ( int x = 0 ; x < w ; ++x ) {
                    pxl_Red[x].green = pxl_Red[x].blue = pxl_Red[x].red;
                    pxl_Green[x].red = pxl_Green[x].blue = pxl_Green[x].green;
                    pxl_Blue[x].red = pxl_Blue[x].green = pxl_Blue[x].blue;
                }
            }
            iRed_cache.sync();
            iGreen_cache.sync();
            iBlue_cache.sync();
            m_outputs[0].push_back(pLuminance);
            m_outputs[1].push_back(pRed);
            m_outputs[2].push_back(pGreen);
            m_outputs[3].push_back(pBlue);
            emitProgress(p, c, 1, 1);
            ++p;
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }
};

OpRGBDecompose::OpRGBDecompose(Process *parent) :
    Operator(OP_SECTION_COLOR, "LRGB Decompose", parent)
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
