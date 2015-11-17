#include "operatorworker.h"
#include "operatorrotate.h"
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
    qreal angle = dynamic_cast<OperatorRotate*>(m_operator)->angle();

    qWarning(QString("apply rotation of %0 °").arg(angle).toLatin1());
    newPhoto.image().rotate(angle);
    return newPhoto;
}

OperatorRotate::OperatorRotate(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, "Rotation", parent),
    m_dropdown(new OperatorParameterDropDown("angle","angle","0°",this)),
    m_angle(0)
{
    m_dropdown->addOption("0°", this, SLOT(set0()));
    m_dropdown->addOption("90°", this, SLOT(set90()));
    m_dropdown->addOption("180°", this, SLOT(set180()));
    m_dropdown->addOption("270°", this, SLOT(set270()));
    m_parameters.push_back(m_dropdown);
    m_inputs.push_back(new OperatorInput("Images","Image", OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Rotated", "Rotated", this));
}

OperatorRotate::~OperatorRotate()
{
}

OperatorRotate *OperatorRotate::newInstance()
{
    return new OperatorRotate(m_process);
}

OperatorWorker *OperatorRotate::newWorker()
{
    return new RotateWorker(m_thread, this);
}

void OperatorRotate::set0()
{
    if ( m_angle != 0 ) {
        m_angle = 0;
        setUpToDate(false);
    }
}

void OperatorRotate::set90()
{
    if ( m_angle != 90 ) {
        m_angle = 90;
        setUpToDate(false);
    }
}

void OperatorRotate::set180()
{
    if ( m_angle != 180 ) {
        m_angle = 180;
        setUpToDate(false);
    }
}

void OperatorRotate::set270()
{
    if ( m_angle != 270 ) {
        m_angle = 270;
        setUpToDate(false);
    }
}

qreal OperatorRotate::angle() const
{
    return m_angle;
}
