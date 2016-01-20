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

    dflDebug(QString("apply rotation of %0 °").arg(angle));
    newPhoto.image().rotate(angle);
    return newPhoto;
}

OpRotate::OpRotate(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, QT_TRANSLATE_NOOP("Operator", "Rotation"), Operator::All, parent),
    m_dropdown(new OperatorParameterDropDown("angle","angle",this, SLOT(setAngle(int)))),
    m_angle(0)
{
    m_dropdown->addOption("0°", 0, true);
    m_dropdown->addOption("90°", 90);
    m_dropdown->addOption("180°", 180);
    m_dropdown->addOption("270°", 270);
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

void OpRotate::setAngle(int v)
{
    if ( m_angle != v ) {
        m_angle = v;
        setOutOfDate();
    }
}

qreal OpRotate::angle() const
{
    return m_angle;
}
