/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
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
    Operator(OP_SECTION_WORKFLOW, QT_TRANSLATE_NOOP("Operator", "%0-way Demultiplexer"), Operator::All, parent),
    m_ways(ways)
{
    m_classIdentifier = m_classIdentifier.arg(ways);
    m_name = m_name.arg(ways);
    m_localizedClassIdentifier= m_localizedClassIdentifier.arg(ways);
    addInput(new OperatorInput(tr("Multiplexed set"), OperatorInput::Set, this));
    for (int i = 1 ; i <= m_ways ; ++i ) {
        QString name = tr("Image set %0").arg(i);
        addOutput(new OperatorOutput(name, this));
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
