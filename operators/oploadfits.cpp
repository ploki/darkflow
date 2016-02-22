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
#include "oploadfits.h"
#include "process.h"

#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "workerloadfits.h"

static const char *ColorSpaceStr[] = {
    QT_TRANSLATE_NOOP("OpLoadFits", "Linear"),
    QT_TRANSLATE_NOOP("OpLoadFits", "sRGB"),
    QT_TRANSLATE_NOOP("OpLoadFits", "IUT BT.709")
};

OpLoadFits::OpLoadFits(Process *parent) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Fits Images"), Operator::NA, parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "imageCollection",
                          tr("Images"),
                          tr("Select images to add to the collection"),
                          m_process->baseDirectory(),
                          tr("FITS Images (*.fits *.fit);;"
                          "All Files (*.*)"), this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", tr("Color Space"), this, SLOT(setColorSpace(int)))),
    m_colorSpaceValue(Linear),
    m_outputHDR(new OperatorParameterDropDown("outputHDR",  tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[Linear]), Linear, true);
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[sRGB]), sRGB);
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[IUT_BT_709]), IUT_BT_709);
    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);
    addParameter(m_filesCollection);
    addParameter(m_colorSpace);
    addParameter(m_outputHDR);
    addOutput(new OperatorOutput(tr("Images"),this));
}

OpLoadFits *OpLoadFits::newInstance()
{
    return new OpLoadFits(m_process);
}

OperatorWorker *OpLoadFits::newWorker()
{
    QVector<QString> filesCollection = m_filesCollection->collection().toVector();
    return new WorkerLoadFits(filesCollection, m_colorSpaceValue, m_outputHDRValue, m_thread, this);
}

void OpLoadFits::setColorSpace(int v)
{
    if ( m_colorSpaceValue != v ) {
        m_colorSpaceValue = ColorSpace(v);
        setOutOfDate();
    }
}

void OpLoadFits::setOutputHDR(int v)
{
    if ( m_outputHDRValue != !!v ) {
        m_outputHDRValue = !!v;
        setOutOfDate();
    }
}

void OpLoadFits::filesCollectionChanged()
{
    setOutOfDate();
}
