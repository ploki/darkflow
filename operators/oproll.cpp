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
        newPhoto.image().roll(m_columns, m_rows);
        return newPhoto;
    }

private:
    int m_columns;
    int m_rows;
};

OpRoll::OpRoll(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, "Roll", Operator::All, parent),
    m_columns(new OperatorParameterSlider("columns", "Columns", "Roll Columns", Slider::Value, Slider::Linear, Slider::Integer, -10, 10, 0, -32768, 32768, Slider::FilterPixels, this)),
    m_rows(new OperatorParameterSlider("rows", "Rows", "Roll Rows", Slider::Value, Slider::Linear, Slider::Integer, -10, 10, 0, -32768, 32768, Slider::FilterPixels, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Rolled", "Rolled", this));
    addParameter(m_columns);
    addParameter(m_rows);
}

OpRoll *OpRoll::newInstance()
{
    return new OpRoll(m_process);
}

OperatorWorker *OpRoll::newWorker()
{
    return new WorkerRoll(round(m_columns->value()),
                          round(m_rows->value()),
                          m_thread, this);
}
