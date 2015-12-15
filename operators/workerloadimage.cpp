#include <list>

#include "workerloadimage.h"


WorkerLoadImage::WorkerLoadImage(QVector<QString> filesCollection,
                                 OpLoadImage::ColorSpace colorSpace,
                                 QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_filesCollection(filesCollection),
    m_colorSpace(colorSpace)
{
}

Photo WorkerLoadImage::process(const Photo &, int, int)
{
    throw 0;
}

void WorkerLoadImage::play()
{
    QVector<QString>& collection = m_filesCollection;

    int s = collection.count();
    int p = 0;
    volatile bool failure = false;

#pragma omp parallel for shared(failure)
    for (int i = 0 ; i < s ; ++i) {
        if ( failure || aborted() ) {
            failure = true;
            continue;
        }
        try {
            QString filename=collection[i];
            std::list<Magick::Image> images;
            Magick::readImages(&images, filename.toStdString());
            Photo::Gamma gamma;
            switch(m_colorSpace) {
            default:
            case OpLoadImage::Linear: gamma = Photo::Linear; break;
            case OpLoadImage::IUT_BT_709: gamma = Photo::IUT_BT_709; break;
            case OpLoadImage::sRGB: gamma = Photo::sRGB; break;
            }
            int plane = 0;
            int count = images.size();
            for( std::list<Magick::Image>::iterator it = images.begin() ;
                 it != images.end() ;
                 ++it ) {
                Photo photo(*it, gamma);
                if ( !photo.isComplete() ) {
                    failure = true;
                    continue;
                }
                QString identity = collection[i];
                if ( count > 1 )
                    identity += ":" + QString::number(plane);
                photo.setIdentity(identity);
                //setTags(collection[i], photo);
                photo.setTag("Name", identity);
                photo.setSequenceNumber(i);
#pragma omp critical
                {
                    outputPush(0, photo);
                }
                ++plane;
            }
        }
        catch(Magick::Exception *e) {
            dflError(e->what());
            delete e;
        }
        catch(std::exception *e) {
            dflError(e->what());
            delete e;
        }
        catch(...) {
            dflError("unknown image exception");
        }

#pragma omp critical
        {
            emit progress(++p, s);
        }
    }
    if ( failure ) {
        emitFailure();
    }
    else {
        outputSort(0);
        emitSuccess();
    }

}
