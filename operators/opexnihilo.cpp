#include <QVector>
#include "operatoroutput.h"
#include "opexnihilo.h"
#include "operatorworker.h"
#include "process.h"
#include "photo.h"
#include "console.h"

#include <Magick++.h>

class ExNihilo : public OperatorWorker
{
public:
    ExNihilo(QThread *thread, OpExNihilo *op) :
        OperatorWorker(thread, op)
    {}
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play() {
        dflDebug("play!!");
        Photo photo(Photo::Linear);
        photo.setIdentity(m_operator->uuid());
        photo.createImage(1000,1000);
        if (photo.isComplete()) {
            try {
                Magick::Image& image = photo.image();
                Magick::Pixels cache(image);
                unsigned w = image.columns();
                unsigned h = image.rows();
                for (unsigned y = 0 ; y < h ; ++y) {
                    emit progress(y, h);
                    if ( aborted() ) {
                        emitFailure();
                        return;
                    }
                    Magick::PixelPacket *pixels = cache.get(0,y,w,1);
                    if ( m_error || !pixels ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for (unsigned x = 0 ; x < w ; ++x ) {
                        using Magick::Quantum;
                        pixels[x].red = qrand()%QuantumRange;
                        pixels[x].green = qrand()%QuantumRange;
                        pixels[x].blue = qrand()%QuantumRange;
                    }
                    cache.sync();
                }
                photo.setTag(TAG_NAME, "Random Image");
                outputPush(0, photo);
                emitSuccess();
            }
            catch(std::exception &e) {
                dflError("%s", e.what());
                emitFailure();
            }
        }
        else {
            emitFailure();
        }
    }
};

OpExNihilo::OpExNihilo(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, "Ex Nihilo", Operator::NA, parent)
{
    addOutput(new OperatorOutput("Random image", "Random Image", this));
}

OpExNihilo::~OpExNihilo()
{
    //dflDebug((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}

OpExNihilo *OpExNihilo::newInstance()
{
    return new OpExNihilo(m_process);
}

OperatorWorker *OpExNihilo::newWorker()
{
    return new ExNihilo(m_thread, this);
}

