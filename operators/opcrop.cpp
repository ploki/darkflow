#include <QStringList>

#include "opcrop.h"

#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

class WorkerCrop : public OperatorWorker {
public:
    WorkerCrop(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        QString roiTag = newPhoto.getTag("ROI");
        if ( roiTag.count() == 0 )
            return newPhoto;
        newPhoto.removeTag("ROI");
        QStringList coord = roiTag.split(',');
        if ( coord.size() == 4 ) {
            qreal x1 = coord[0].toDouble();
            qreal y1 = coord[1].toDouble();
            qreal x2 = coord[2].toDouble();
            qreal y2 = coord[3].toDouble();
            if ( x1 < 0 ) x1 = 0;
            if ( y1 < 0 ) y1 = 0;
            if ( x2 < 0 ) x2 = 0;
            if ( y2 < 0 ) y2 = 0;
            if ( x1 > photo.image().columns() ) x1=photo.image().columns();
            if ( y1 > photo.image().rows() ) y1=photo.image().rows();
            if ( x2 > photo.image().columns() ) x2=photo.image().columns();
            if ( y2 > photo.image().rows() ) y2=photo.image().rows();
            qreal x=x1,y=y1,w=x2-x1,h=y2-y1;
            if ( x1 > x2 ) {
                x=x2; w=-w;
            }
            if ( y1 > y2 ) {
                y=y2; h=-h;
            }
            qDebug("x1:%f, y1:%f, x2:%f, y2:%f",x1,y1,x2,y2);
            qDebug("x:%f, y:%f, w:%f, h:%f",x,y,w,h);
            Magick::Geometry geo(w,h,x,y);
            //nasty kludge to prevent crop to miss the target (it's my understanding)
            newPhoto.image().page(Magick::Geometry(0,0,0,0));
            newPhoto.image().crop(geo);
        }
        else {
            newPhoto.setTag("ROI", "##ERROR!");
        }
        return newPhoto;
    }
};

OpCrop::OpCrop(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, "Crop", parent)
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Cropped", "Cropped", this));

}

OpCrop *OpCrop::newInstance()
{
    return new OpCrop(m_process);
}

OperatorWorker *OpCrop::newWorker()
{
    return new WorkerCrop(m_thread, this);
}
