#include "opdemultiplexer.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"


class WorkerDemultiplexer : public OperatorWorker {
public:
    WorkerDemultiplexer(int ways, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_ways(ways)
    {

    }
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int n_outputs = outputsCount();
        Q_ASSERT( n_outputs == m_ways );
        int i = 0,
                s = m_inputs[0].count();
        foreach(Photo photo, m_inputs[0]) {
            if (aborted())
                continue;
            outputPush(i%n_outputs, photo);
            emitProgress(i, s, 0, 1);
            ++i;
        }
        if (aborted() )
            emitFailure();
        else
            emitSuccess();

    }
private:
    int m_ways;
};

OpDemultiplexer::OpDemultiplexer(int ways, Process *parent) :
    Operator(OP_SECTION_TOOLS, QString("%0-way Demultiplexer").arg(ways), Operator::All, parent),
    m_ways(ways)
{
    addInput(new OperatorInput("Multiplexed set", "Multiplexed set", OperatorInput::Set, this));
    for (int i = 1 ; i <= m_ways ; ++i ) {
        QString name = QString("Images set %0").arg(i);
        addOutput(new OperatorOutput(name, name, this));
    }
}

OpDemultiplexer *OpDemultiplexer::newInstance()
{
    return new OpDemultiplexer(m_ways, m_process);
}

OperatorWorker *OpDemultiplexer::newWorker()
{
    return new WorkerDemultiplexer(m_ways, m_thread, this);
}
