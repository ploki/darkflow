#include "oprgbcompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "cielab.h"
#include "igamma.h"

class WorkerRGBCompose : public OperatorWorker {
public:
    WorkerRGBCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play(QVector<QVector<Photo> > inputs, int n_outputs) {
        m_inputs = inputs;
        play_prepareOutputs(n_outputs);
        int lab_count = inputs[0].count();
        int photo_count = inputs[1].count();
        for (int i = 2 ; i < 4 ; ++i )
            if ( inputs[i].count() != photo_count ) {
                qWarning("Uneven photo count in LRGB Compose");
                emitFailure();
                return;
            }
        if ( lab_count && lab_count != photo_count ) {
            qWarning("Wrong number of Luminance photos in LRGB Compose");
            emitFailure();
            return;
        }
        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pRed(inputs[1][i]);
            Photo pGreen(inputs[2][i]);
            Photo pBlue(inputs[3][i]);
            Magick::Image& iRed = pRed.image();
            Magick::Image& iGreen = pGreen.image();
            Magick::Image& iBlue = pBlue.image();
            iRed.modifyImage();
            Magick::Pixels iRed_cache(iRed);
            Magick::Pixels iGreen_cache(iGreen);
            Magick::Pixels iBlue_cache(iBlue);
            int w = iRed.columns();
            int h = iRed.rows();
#pragma omp parallel for
            for ( int y = 0 ; y < h ; ++y ) {
                Magick::PixelPacket *pxl_Red = iRed_cache.get(0, y, w, 1);
                const Magick::PixelPacket *pxl_Green = iGreen_cache.getConst(0, y, w, 1);
                const Magick::PixelPacket *pxl_Blue = iBlue_cache.getConst(0, y, w, 1);
                for ( int x = 0 ; x < w ; ++x ) {
                    pxl_Red[x].green = pxl_Green[x].green;
                    pxl_Red[x].blue = pxl_Blue[x].blue;
                }
            }
            iRed_cache.sync();
            if ( lab_count ) {
                Photo lPhoto(inputs[0][i]);
                iGamma& labGamma = iGamma::Lab();
                labGamma.applyOn(lPhoto);
                unLabize(pRed.image(), lPhoto.image());
            }
            m_outputs[0].push_back(pRed);
            emitProgress(i, photo_count, 1, 1);
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
    m_inputs.push_back(new OperatorInput("Luminance", "Luminance", OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Red", "Red", OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Green", "Green", OperatorInput::Set, this));
    m_inputs.push_back(new OperatorInput("Blue", "Blue", OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("RGB", "RGB", this));
}

OpRGBCompose *OpRGBCompose::newInstance()
{
    return new OpRGBCompose(m_process);
}

OperatorWorker *OpRGBCompose::newWorker()
{
    return new WorkerRGBCompose(m_thread, this);
}
