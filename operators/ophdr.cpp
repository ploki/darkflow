#include "ophdr.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "hdr.h"

class WorkerHDR : public OperatorWorker {
public:
    WorkerHDR(bool invert, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_hdr(invert),
        m_invert(invert)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        if ( m_invert && photo.getScale() != Photo::HDR )
            dflWarning(tr("%0: must be HDR").arg(photo.getIdentity()));
        if ( !m_invert && photo.getScale() != Photo::Linear)
            dflWarning(tr("%0: must be Linear").arg(photo.getIdentity()));
        m_hdr.applyOn(newPhoto);
        return newPhoto;
    }
private:
    HDR m_hdr;
    bool m_invert;
};

OpHDR::OpHDR(Process *parent) :
    Operator(OP_SECTION_CURVE, QT_TRANSLATE_NOOP("Operator", "HDR"), Operator::HDR|Operator::Linear, parent),
    m_revert(false),
    m_revertDialog(new OperatorParameterDropDown("revert", tr("Revert"), this, SLOT(revert(int))))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));
    m_revertDialog->addOption(DF_TR_AND_C("No"), false, true);
    m_revertDialog->addOption(DF_TR_AND_C("Yes"), true);
    addParameter(m_revertDialog);

}

OpHDR *OpHDR::newInstance()
{
    return new OpHDR(m_process);
}

OperatorWorker *OpHDR::newWorker()
{
    return new WorkerHDR(m_revert, m_thread, this);
}

void OpHDR::revert(int v)
{
    if ( m_revert != !!v ) {
        m_revert = !!v;
        setOutOfDate();
    }
}
