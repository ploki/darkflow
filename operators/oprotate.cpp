#include "operatorworker.h"
#include "oprotate.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "photo.h"
#include <Magick++.h>

class RotateWorker : public OperatorWorker {
public:
    RotateWorker(QThread *thread, Operator *op) :
        OperatorWorker(thread, op) {}
    Photo process(const Photo& Photo, int p, int c) Q_DECL_OVERRIDE;
};

Photo RotateWorker::process(const Photo& photo, int p, int c) {
    Q_UNUSED(p);
    Q_UNUSED(c);
    Photo newPhoto(photo);
    qreal angle = dynamic_cast<OpRotate*>(m_operator)->angle();

    qDebug(QString("apply rotation of %0 °").arg(angle).toLatin1());
    newPhoto.image().rotate(angle);
    return newPhoto;
}

OpRotate::OpRotate(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, "Rotation", parent),
    m_dropdown(new OperatorParameterDropDown("angle","angle","0°",this)),
    m_angle(0)
{
    m_dropdown->addOption("0°", this, SLOT(set0()));
    m_dropdown->addOption("90°", this, SLOT(set90()));
    m_dropdown->addOption("180°", this, SLOT(set180()));
    m_dropdown->addOption("270°", this, SLOT(set270()));
    addParameter(m_dropdown);
    addInput(new OperatorInput("Images","Image", OperatorInput::Set, this));
    addOutput(new OperatorOutput("Rotated", "Rotated", this));
}

OpRotate::~OpRotate()
{
}

OpRotate *OpRotate::newInstance()
{
    return new OpRotate(m_process);
}

OperatorWorker *OpRotate::newWorker()
{
    return new RotateWorker(m_thread, this);
}

void OpRotate::set0()
{
    if ( m_angle != 0 ) {
        m_angle = 0;
        setOutOfDate();
    }
}

void OpRotate::set90()
{
    if ( m_angle != 90 ) {
        m_angle = 90;
        setOutOfDate();
    }
}

void OpRotate::set180()
{
    if ( m_angle != 180 ) {
        m_angle = 180;
        setOutOfDate();
    }
}

void OpRotate::set270()
{
    if ( m_angle != 270 ) {
        m_angle = 270;
        setOutOfDate();
    }
}

qreal OpRotate::angle() const
{
    return m_angle;
}
