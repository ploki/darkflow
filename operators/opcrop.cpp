#include <QStringList>
#include <QRectF>

#include "opcrop.h"

#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

class WorkerCrop : public OperatorWorker {
public:
    WorkerCrop(QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_refROI()
    {}
    void play_analyseSources() {
        foreach(Photo photo, m_inputs[0]) {
            m_refROI = photo.getROI();
            if ( !m_refROI.isNull() )
                break;
        }
    }

    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        QRectF roi = m_refROI;
        newPhoto.removeTag(TAG_ROI);
        if ( roi.isNull() )
            roi = newPhoto.getROI();
        if ( roi.isNull() ) {
            setError(photo, TAG_ROI" not defined");
            return newPhoto;
        }
        if ( roi.height() < 1 || roi.width() < 1 ) {
            setError(photo, TAG_ROI" too small");
            return newPhoto;
        }
        Magick::Geometry geo(roi.width(),roi.height(),roi.x(),roi.y());
        //nasty kludge to prevent crop to miss the target (it's my understanding)
        newPhoto.image().page(Magick::Geometry(0,0,0,0));
        newPhoto.image().crop(geo);
        return newPhoto;
    }
private:
    QRectF m_refROI;
};

OpCrop::OpCrop(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, QT_TRANSLATE_NOOP("Operator", "Crop"), Operator::All, parent)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Cropped"), this));

}

OpCrop *OpCrop::newInstance()
{
    return new OpCrop(m_process);
}

OperatorWorker *OpCrop::newWorker()
{
    return new WorkerCrop(m_thread, this);
}
