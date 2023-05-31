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
#include "opdebayer.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerdebayer.h"

static const char *DebayerStr[] = {
    QT_TRANSLATE_NOOP("OpDebayer", "None"),
    QT_TRANSLATE_NOOP("OpDebayer", "Mask"),
    QT_TRANSLATE_NOOP("OpDebayer", "HalfSize"),
    QT_TRANSLATE_NOOP("OpDebayer", "Simple"),
    QT_TRANSLATE_NOOP("OpDebayer", "Bilinear"),
    QT_TRANSLATE_NOOP("OpDebayer", "HQ Linear"),
    //QT_TRANSLATE_NOOP("OpDebayer", "Down Sample"),
    QT_TRANSLATE_NOOP("OpDebayer", "VNG"),
    QT_TRANSLATE_NOOP("OpDebayer", "AHD")
};

static const char *FilterPatternStr[] = {
    QT_TRANSLATE_NOOP("OpLoadImage", "Default"),
    QT_TRANSLATE_NOOP("OpLoadImage", "BG/GR"),
    QT_TRANSLATE_NOOP("OpLoadImage", "RG/GB"),
    QT_TRANSLATE_NOOP("OpLoadImage", "GB/RG"),
    QT_TRANSLATE_NOOP("OpLoadImage", "GR/BG"),
};

OpDebayer::OpDebayer(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "Debayer"), Operator::All, parent),
    m_debayer(new OperatorParameterDropDown("quality", tr("Quality"), this, SLOT(setDebayer(int)))),
    m_debayerValue(Bilinear),
    m_filterPattern(new OperatorParameterDropDown("filterPattern", tr("Filter Pattern"), this, SLOT(setFilterPattern(int)))),
    m_filterPatternValue(Default)
{
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[NoDebayer]), NoDebayer);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Mask]), Mask);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[HalfSize]), HalfSize);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Simple]), Simple);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Bilinear]), Bilinear);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[HQLinear]), HQLinear, true);
    //m_debayer->addOption(DF_TR_AND_C(DebayerStr[DownSample]), DownSample);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[VNG]), VNG);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[AHD]), AHD);
    m_filterPattern->addOption(DF_TR_AND_C(FilterPatternStr[Default]), Default, true);
    m_filterPattern->addOption(DF_TR_AND_C(FilterPatternStr[BGGR]), BGGR);
    m_filterPattern->addOption(DF_TR_AND_C(FilterPatternStr[RGGB]), RGGB);
    m_filterPattern->addOption(DF_TR_AND_C(FilterPatternStr[GBRG]), GBRG);
    m_filterPattern->addOption(DF_TR_AND_C(FilterPatternStr[GRBG]), GRBG);
    addParameter(m_debayer);
    addParameter(m_filterPattern);
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Images"), this));

}

void OpDebayer::setDebayer(int v)
{
    if ( m_debayerValue != v ) {
        m_debayerValue = Debayer(v);
        setOutOfDate();
    }
}

OpDebayer *OpDebayer::newInstance()
{
    return new OpDebayer(m_process);
}

OperatorWorker *OpDebayer::newWorker()
{
    return new WorkerDebayer(m_debayerValue, m_thread, m_filterPatternValue, this);
}

void OpDebayer::setFilterPattern(int v)
{
    if ( m_filterPatternValue != v) {
        m_filterPatternValue = FilterPattern(v);
        setOutOfDate();
    }
}
