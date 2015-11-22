#include <QVector>
#include "operatoroutput.h"
#include "operatorexnihilo.h"
#include "operatorworker.h"
#include "process.h"
#include "photo.h"

#include <Magick++.h>

class ExNihilo : public OperatorWorker
{
public:
    ExNihilo(QThread *thread, OperatorExNihilo *op) :
        OperatorWorker(thread, op)
    {}
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play(QVector<QVector<Photo> > inputs, int n_outputs) {
        qDebug("play!!");
        Photo photo(Photo::Linear);
        m_inputs = inputs;
        play_prepareOutputs(n_outputs);
        photo.setIdentity(m_operator->uuid());
        photo.createImage(1000,1000);
        if (photo.isComplete()) {
            Magick::Image& image = photo.image();
            image.modifyImage();
            Magick::Pixels cache(image);
            size_t w = image.columns();
            size_t h = image.rows();
            for (size_t y = 0 ; y < h ; ++y) {
                emit progress(y, h);
                if ( aborted() ) {
                    emitFailure();
                    return;
                }
                Magick::PixelPacket *pixels = cache.get(0,y,w,1);
                for (size_t x = 0 ; x < w ; ++x ) {
                    using Magick::Quantum;
                    pixels[x].red = qrand()%QuantumRange;
                    pixels[x].green = qrand()%QuantumRange;
                    pixels[x].blue = qrand()%QuantumRange;
                }
            }
            cache.sync();
            photo.setTag("Name", "Random Image");
            m_outputs[0].push_back(photo);
            emitSuccess();
        }
        else {
            emitFailure();
        }
    }
};

OperatorExNihilo::OperatorExNihilo(Process *parent) :
    Operator(OP_SECTION_UTILITY, "Ex Nihilo", parent)
{
    m_outputs.push_back(new OperatorOutput("Random image", "Random Image", this));
}

OperatorExNihilo::~OperatorExNihilo()
{
    //qDebug((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}

OperatorExNihilo *OperatorExNihilo::newInstance()
{
    return new OperatorExNihilo(m_process);
}

OperatorWorker *OperatorExNihilo::newWorker()
{
    return new ExNihilo(m_thread, this);
}

