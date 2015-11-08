#include <QVector>
#include "operatoroutput.h"
#include "operatorexnihilo.h"
#include "operatorworker.h"
#include "process.h"
#include "photo.h"
#include <unistd.h>

#include <Magick++.h>

class ExNihilo : public OperatorWorker
{
public:
    ExNihilo(QThread *thread, OperatorExNihilo *op) :
        OperatorWorker(thread, op)
    {}
private slots:
    void play() {
        qWarning("play!!");
        Photo photo;
        photo.create(1000,1000);
        if (!photo.error()) {
            Magick::Image *image = photo.image();
            image->modifyImage();
            Magick::Pixels cache(*image);
            size_t w = image->columns();
            size_t h = image->rows();
            for (size_t y = 0 ; y < h ; ++y) {
                emit progress(y, h);
                if ( aborted() ) {
                    emitFailure();
                    return;
                }
                usleep(1000);
                Magick::PixelPacket *pixels = cache.get(0,y,w,1);
                for (size_t x = 0 ; x < w ; ++x ) {
                    pixels[x].red = 12;
                    pixels[x].green = 128;
                    pixels[x].blue = 250;
                }
            }
            cache.sync();
            m_operator->m_outputs[0]->m_result.push_back(photo);
            emitSuccess();
        }
        else {
            emitFailure();
        }
    }
};

OperatorExNihilo::OperatorExNihilo(Process *parent) :
    Operator("Ex Nihilo", parent)
{
    m_outputs.push_back(new OperatorOutput("Random image", "Random Image", this));
}

OperatorExNihilo::~OperatorExNihilo()
{
    //qWarning((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}

OperatorExNihilo *OperatorExNihilo::newInstance()
{
    return new OperatorExNihilo(m_process);
}

OperatorWorker *OperatorExNihilo::newWorker()
{
    return new ExNihilo(m_thread, this);
}

