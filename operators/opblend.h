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
#ifndef OPBLEND_H
#define OPBLEND_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;

class OpBlend : public Operator
{
    Q_OBJECT
public:
    OpBlend(Process *parent);
    OpBlend *newInstance();
    OperatorWorker *newWorker();

    typedef enum {
        Multiply,
        Screen,
        Overlay,
        HardLight,
        SoftLight,
        DivideBrighten,
        Divide,
        DivideDarken,
        Addition,
        Subtract,
        Difference,
        DarkenOnly,
        LightenOnly
    }   BlendMode;

public slots:
    void selectMode1(int v);
    void selectMode2(int v);
    void setOutputHDR(int type);
private:
    OperatorParameterDropDown *m_mode1;
    OperatorParameterDropDown *m_mode2;
    BlendMode m_mode1Value;
    BlendMode m_mode2Value;
    OperatorParameterDropDown *m_outputHDR;
    bool m_outputHDRValue;

    void registerOptions(OperatorParameterDropDown *mode);
};

#endif // OPBLEND_H
