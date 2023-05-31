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
#include "opblend.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerblend.h"

static const char *BlendModeStr[] = {
    QT_TRANSLATE_NOOP("OpBlend", "Multiply"), QT_TRANSLATE_NOOP("OpBlend", "Screen"), QT_TRANSLATE_NOOP("OpBlend", "Overlay"), QT_TRANSLATE_NOOP("OpBlend", "Hard Light"),
    QT_TRANSLATE_NOOP("OpBlend", "Soft Light"), QT_TRANSLATE_NOOP("OpBlend", "Divide Brighten"), QT_TRANSLATE_NOOP("OpBlend", "Divide"), QT_TRANSLATE_NOOP("OpBlend", "Divide Darken"),
    QT_TRANSLATE_NOOP("OpBlend", "Addition"), QT_TRANSLATE_NOOP("OpBlend", "Subtract"), QT_TRANSLATE_NOOP("OpBlend", "Difference"),
    QT_TRANSLATE_NOOP("OpBlend", "Darken Only"), QT_TRANSLATE_NOOP("OpBlend", "Lighten Only")
};


OpBlend::OpBlend(Process *parent) :
    Operator(OP_SECTION_BLEND, QT_TRANSLATE_NOOP("Operator", "Arithmetic"), Operator::All, parent),
    m_mode1(new OperatorParameterDropDown("mode1", tr("A « B"), this, SLOT(selectMode1(int)))),
    m_mode2(new OperatorParameterDropDown("mode2", tr("AB « C"), this, SLOT(selectMode2(int)))),
    m_mode1Value(Multiply),
    m_mode2Value(Multiply),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    //previous name was Blend...
    //here for compat with existing projects
    m_classIdentifier = "Blend";
    registerOptions(m_mode1);
    registerOptions(m_mode2);
    addParameter(m_mode1);
    addParameter(m_mode2);

    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);

    addInput(new OperatorInput(tr("Layer A (top)"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Layer B (Middle)"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Layer C (Bottom)"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Blend"), this));
    addOutput(new OperatorOutput(tr("Overflow"), this));
    addOutput(new OperatorOutput(tr("Underflow"), this));
    addParameter(m_outputHDR);
}

void OpBlend::registerOptions(OperatorParameterDropDown *mode)
{
    mode->addOption(DF_TR_AND_C(BlendModeStr[Multiply]), Multiply, true);
    mode->addOption(DF_TR_AND_C(BlendModeStr[Screen]), Screen);
    //mode->addOption(DF_TR_AND_C(BlendModeStr[Overlay]), Overlay);
    //mode->addOption(DF_TR_AND_C(BlendModeStr[HardLight]), HardLight);
    //mode->addOption(DF_TR_AND_C(BlendModeStr[SoftLight]), SoftLight);
    mode->addOption(DF_TR_AND_C(BlendModeStr[DivideBrighten]), DivideBrighten);
    mode->addOption(DF_TR_AND_C(BlendModeStr[Divide]), Divide);
    mode->addOption(DF_TR_AND_C(BlendModeStr[DivideDarken]), DivideDarken);
    mode->addOption(DF_TR_AND_C(BlendModeStr[Addition]), Addition);
    mode->addOption(DF_TR_AND_C(BlendModeStr[Subtract]), Subtract);
    mode->addOption(DF_TR_AND_C(BlendModeStr[Difference]), Difference);
    mode->addOption(DF_TR_AND_C(BlendModeStr[DarkenOnly]), DarkenOnly);
    mode->addOption(DF_TR_AND_C(BlendModeStr[LightenOnly]), LightenOnly);
}

OpBlend *OpBlend::newInstance()
{
    return new OpBlend(m_process);
}

OperatorWorker *OpBlend::newWorker()
{
    dflDebug(tr("mode1: %0, mode2: %1").arg(m_mode1Value).arg(m_mode2Value));
    return new WorkerBlend(m_mode1Value, m_mode2Value, m_outputHDRValue, m_thread, this);
}

void OpBlend::selectMode1(int v)
{
    if ( m_mode1Value != v ) {
        m_mode1Value = BlendMode(v);
        setOutOfDate();
    }
}

void OpBlend::selectMode2(int v)
{
    if ( m_mode2Value != v ) {
        m_mode2Value = BlendMode(v);
        setOutOfDate();
    }
}
void OpBlend::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}

