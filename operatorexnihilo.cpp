#include <QVector>
#include "operatoroutput.h"
#include "operatorexnihilo.h"
#include "operatorworker.h"
#include "process.h"
#include "image.h"
#include <unistd.h>

class ExNihilo : public OperatorWorker
{
public:
    ExNihilo(QThread *thread, OperatorExNihilo *op) :
        OperatorWorker(thread, op)
    {}
private slots:
    void play() {
        Image *image = process(NULL);
        if (image) {
            m_operator->m_outputs[0]->m_result.push_back(image);
            emitSuccess();
        }

    }
    Image *process(const Image *image) {
        Q_UNUSED(image);
        QString filename = m_operator->m_process->temporaryDirectory() + "/";
        filename += Image::NewRandomName();
        Image *newImage = new Image(filename);
        newImage->create(1000,1000,Image::sRGB, Image::Pixel_u16);
        Image::Triplet<Image::u16> *pixels =
                newImage->getPixelsTriplet<Image::u16>();
        for (int y = 0 ; y < newImage->height() ; ++y) {
            emit progress(y, newImage->height());
            if ( aborted() ) {
                emitFailure();
                return NULL;
            }
            usleep(1000);
            for (int x = 0 ; x < newImage->width() ; ++x ) {
                pixels[y*newImage->width()+x] = Image::Triplet<Image::u16>(12,128,250);
            }
        }
        newImage->save();
        return newImage;

    }
};

OperatorExNihilo::OperatorExNihilo(Process *parent) :
    Operator(parent)
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

QString OperatorExNihilo::getClassIdentifier()
{
    return "Ex Nihilo";
}

OperatorWorker *OperatorExNihilo::newWorker()
{
    return new ExNihilo(m_thread, this);
}

