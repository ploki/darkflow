#include "opmultiplexer.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"

class WorkerMultiplexer : public OperatorWorker {
public:
    WorkerMultiplexer(int ways, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_ways(ways)
    {

    }
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play(QVector<QVector<Photo> > inputs, int n_outputs) {
        m_inputs = inputs;
        play_prepareOutputs(n_outputs);
        Q_ASSERT( inputs.count() == m_ways );
        int photo_count = inputs[0].count();
        for (int i = 1 ; i < inputs.count() ; ++i )
            if ( inputs[i].count() != photo_count ) {
                qWarning("Uneven photo count in multiplexer");
                emitFailure();
                return;
            }
        for ( int i = 0 ; i < photo_count ; ++i ) {
            for ( int j = 0 ; j < m_ways ; ++j ) {
                if (aborted())
                    continue;
                m_outputs[0].push_back(inputs[j][i]);
                emitProgress(j, m_ways, i, photo_count);
            }
        }
        if (aborted() )
            emitFailure();
        else
            emitSuccess();
    }

private:
    int m_ways;
};

OpMultiplexer::OpMultiplexer(int ways, Process *parent) :
    Operator(OP_SECTION_TOOLS, QString("%0-way Multiplexer").arg(ways),parent),
    m_ways(ways)
{
    for (int i = 1 ; i <= m_ways ; ++i ) {
        QString name = QString("Images set %0").arg(i);
         addInput(new OperatorInput(name, name, OperatorInput::Set, this));
    }
    addOutput(new OperatorOutput("Multiplexed set", "Multiplexed set", this));
}

OpMultiplexer *OpMultiplexer::newInstance()
{
    return new OpMultiplexer(m_ways, m_process);
}

OperatorWorker *OpMultiplexer::newWorker()
{
    return new WorkerMultiplexer(m_ways, m_thread, this);
}
