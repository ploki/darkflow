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
#ifndef OPERATORPARAMETERSELECTIVELAB_H
#define OPERATORPARAMETERSELECTIVELAB_H

#include "operatorparameter.h"
#include "selectivelab.h"

class OperatorParameterSelectiveLab : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterSelectiveLab(
            const QString &name,
            const QString &caption,
            const QString &windowCaption,
            int hue,
            int coverage,
            bool strict,
            int level,
            bool clipToGamut,
            bool displayGuide,
            bool previewEffect,
            Operator *op);

    QJsonObject save(const QString& baseDirStr);
    void load(const QJsonObject &obj);

    QString currentValue() const;

    QString windowCaption() const;
    void setWindowCaption(const QString &windowCaption);

    int hue() const;
    void setHue(int hue);

    int coverage() const;
    void setCoverage(int coverage);

    bool strict() const;
    void setStrict(bool strict);

    int level() const;
    void setLevel(int level);

    bool clipToGamut() const;
    void setClipToGamut(bool clipToGamut);

    bool displayGuide() const;
    void setDisplayGuide(bool displayGuide);

    bool previewEffect() const;
    void setPreviewEffect(bool previewEffect);

signals:
    void updated();
private:
    QString m_windowCaption;
    int m_hue;
    int m_coverage;
    bool m_strict;
    int m_level;
    bool m_clipToGamut;
    bool m_displayGuide;
    bool m_previewEffect;

};

#endif // OPERATORPARAMETERSELECTIVELAB_H
