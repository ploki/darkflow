#include "oprgbdecompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "console.h"
#include "cielab.h"

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
        foreach(Photo photo, m_inputs[0]) {
            if (aborted())
                continue;
            Photo pRed(photo);
            Photo pGreen(photo);
            Photo pBlue(photo);
            Photo pLuminance(photo);
            Magick::Image& srcImage = photo.image();
            Magick::Image& iRed = pRed.image();
            Magick::Image& iGreen = pGreen.image();
            Magick::Image& iBlue = pBlue.image();
            Magick::Image& iLuminance = pLuminance.image();

            try {
                ResetImage(iRed);
                ResetImage(iGreen);
                ResetImage(iBlue);
                ResetImage(iLuminance);
                Magick::Pixels src_cache(srcImage);
                Magick::Pixels iRed_cache(iRed);
                Magick::Pixels iGreen_cache(iGreen);
                Magick::Pixels iBlue_cache(iBlue);
                Magick::Pixels iLuminance_cache(iLuminance);
                int w = srcImage.columns();
                int h = srcImage.rows();
                int line = 0;
#pragma omp parallel for dfl_threads(4, srcImage, iRed, iGreen, iBlue, iLuminance)
                for ( int y = 0 ; y < h ; ++y ) {
                    const Magick::PixelPacket *src = src_cache.getConst(0, y, w, 1);
                    Magick::PixelPacket *pxl_Red = iRed_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Green = iGreen_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Blue = iBlue_cache.get(0, y, w, 1);
                    Magick::PixelPacket *pxl_Luminance = iLuminance_cache.get(0, y, w, 1);
                    if ( m_error || !src || !pxl_Red || !pxl_Green || !pxl_Blue || !pxl_Luminance ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( int x = 0 ; x < w ; ++x ) {
                        pxl_Red[x].red = pxl_Red[x].green = pxl_Red[x].blue = src[x].red;
                        pxl_Green[x].red = pxl_Green[x].green = pxl_Green[x].blue = src[x].green;
                        pxl_Blue[x].red = pxl_Blue[x].green = pxl_Blue[x].blue = src[x].blue;
                        pxl_Luminance[x].red =
                        pxl_Luminance[x].green =
                        pxl_Luminance[x].blue = DF_ROUND(LUMINANCE_PIXEL(src[x]));
                    }
                    iRed_cache.sync();
                    iGreen_cache.sync();
                    iBlue_cache.sync();
                    iLuminance_cache.sync();
#pragma omp critical
                    {
                        emitProgress(p, c, line++, h);
                    }
                }
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
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "LRGB Decompose"), Operator::NonHDR, parent)
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
