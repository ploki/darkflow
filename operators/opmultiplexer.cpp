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
#include <QObject>
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
    ~WorkerMultiplexer() {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        Q_ASSERT( m_inputs.count() == m_ways );
        int photo_count = m_inputs[0].count();
        for (int i = 1 ; i < m_inputs.count() ; ++i )
            if ( m_inputs[i].count() != photo_count ) {
                dflError(tr("Uneven photo count in multiplexer"));
                emitFailure();
                return;
            }
        for ( int i = 0 ; i < photo_count ; ++i ) {
            for ( int j = 0 ; j < m_ways ; ++j ) {
                if (aborted())
                    continue;
                outputPush(0, m_inputs[j][i]);
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
    Operator(OP_SECTION_WORKFLOW, QT_TRANSLATE_NOOP("Operator", "%0-way Multiplexer"), Operator::All, parent),
    m_ways(ways)
{
    m_classIdentifier = m_classIdentifier.arg(ways);
    m_name = m_name.arg(ways);
    for (int i = 1 ; i <= m_ways ; ++i ) {
        QString name = tr("Image set %0").arg(i);
         addInput(new OperatorInput(name, OperatorInput::Set, this));
    }
    addOutput(new OperatorOutput(tr("Multiplexed set"), this));
}

OpMultiplexer *OpMultiplexer::newInstance()
{
    return new OpMultiplexer(m_ways, m_process);
}

OperatorWorker *OpMultiplexer::newWorker()
{
    return new WorkerMultiplexer(m_ways, m_thread, this);
}
