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
