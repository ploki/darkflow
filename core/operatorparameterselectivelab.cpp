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
#include "operatorparameterselectivelab.h"
#include "console.h"


OperatorParameterSelectiveLab::OperatorParameterSelectiveLab(const QString &name,
                                                             const QString &caption,
                                                             const QString &windowCaption,
                                                             int hue,
                                                             int coverage,
                                                             bool strict,
                                                             int level,
                                                             bool clipToGamut,
                                                             bool displayGuide,
                                                             bool previewEffect,
                                                             Operator *op) :
    OperatorParameter(name, caption, op),
    m_windowCaption(windowCaption),
    m_hue(hue),
    m_coverage(coverage),
    m_strict(strict),
    m_level(level),
    m_clipToGamut(clipToGamut),
    m_displayGuide(displayGuide),
    m_previewEffect(previewEffect)
{

}

QJsonObject OperatorParameterSelectiveLab::save(const QString&)
{
    QJsonObject obj;
    obj["type"] = QString("selectiveLab");
    obj["name"] = m_name;
    obj["hue"] = m_hue;
    obj["coverage"] = m_coverage;
    obj["strict"] = m_strict;
    obj["level"] = m_level;
    obj["clipToGamut"] = m_clipToGamut;
    obj["displayGuide"] = m_displayGuide;
    obj["previewEffect"] = m_previewEffect;
    return obj;
}

void OperatorParameterSelectiveLab::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "selectiveLab" ) {
        dflWarning(tr("SelectiveLab: invalid parameter type"));
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        dflWarning(tr("SelectiveLab: invalid parameter name"));
        return;
    }
    m_hue = obj["hue"].toInt();
    m_coverage = obj["coverage"].toInt();
    m_strict = obj["strict"].toBool();
    m_level = obj["level"].toInt();
    m_clipToGamut = obj["clipToGamut"].toBool();
    m_displayGuide = obj["displayGuide"].toBool();
    m_previewEffect = obj["previewEffect"].toBool();

    emit updated();
}

QString OperatorParameterSelectiveLab::currentValue() const
{
    return QString("%0°/%1°").arg(m_hue).arg(m_coverage);
}

QString OperatorParameterSelectiveLab::windowCaption() const
{
    return m_windowCaption;
}

void OperatorParameterSelectiveLab::setWindowCaption(const QString &windowCaption)
{
    m_windowCaption = windowCaption;
}

int OperatorParameterSelectiveLab::hue() const
{
    return m_hue;
}

void OperatorParameterSelectiveLab::setHue(int hue)
{
    m_hue = hue;
    emit setOutOfDate();
}

int OperatorParameterSelectiveLab::coverage() const
{
    return m_coverage;
}

void OperatorParameterSelectiveLab::setCoverage(int coverage)
{
    m_coverage = coverage;
    emit setOutOfDate();
}

bool OperatorParameterSelectiveLab::strict() const
{
    return m_strict;
}

void OperatorParameterSelectiveLab::setStrict(bool strict)
{
    m_strict = strict;
    emit setOutOfDate();
}

int OperatorParameterSelectiveLab::level() const
{
    return m_level;
}

void OperatorParameterSelectiveLab::setLevel(int level)
{
    m_level = level;
}

bool OperatorParameterSelectiveLab::clipToGamut() const
{
    return m_clipToGamut;
}

void OperatorParameterSelectiveLab::setClipToGamut(bool clipToGamut)
{
    m_clipToGamut = clipToGamut;
}

bool OperatorParameterSelectiveLab::displayGuide() const
{
    return m_displayGuide;
}

void OperatorParameterSelectiveLab::setDisplayGuide(bool displayGuide)
{
    m_displayGuide = displayGuide;
}

bool OperatorParameterSelectiveLab::previewEffect() const
{
    return m_previewEffect;
}

void OperatorParameterSelectiveLab::setPreviewEffect(bool previewEffect)
{
    m_previewEffect = previewEffect;
}
