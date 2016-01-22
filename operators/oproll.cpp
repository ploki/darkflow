#include "oproll.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

class WorkerRoll : public OperatorWorker {
public:
    WorkerRoll(int columns, int rows, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_columns(columns),
        m_rows(rows)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().page(Magick::Geometry(0,0,0,0));
        newPhoto.image().roll(m_columns, m_rows);
        return newPhoto;
    }

private:
    int m_columns;
    int m_rows;
};

OpRoll::OpRoll(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, QT_TRANSLATE_NOOP("Operator", "Roll"), Operator::All, parent),
    m_columns(new OperatorParameterSlider("columns", tr("Columns"), tr("Roll Columns"), Slider::Value, Slider::Linear, Slider::Integer, -10, 10, 0, -32768, 32768, Slider::FilterPixels, this)),
    m_rows(new OperatorParameterSlider("rows", tr("Rows"), tr("Roll Rows"), Slider::Value, Slider::Linear, Slider::Integer, -10, 10, 0, -32768, 32768, Slider::FilterPixels, this))
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Rolled"), this));
    addParameter(m_columns);
    addParameter(m_rows);
}

OpRoll *OpRoll::newInstance()
{
    return new OpRoll(m_process);
}

OperatorWorker *OpRoll::newWorker()
{
    return new WorkerRoll(DF_ROUND(m_columns->value()),
                          DF_ROUND(m_rows->value()),
                          m_thread, this);
}
