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
#ifndef OPLINE_H
#define OPLINE_H

#include "operator.h"
#include <QObject>

class OperatorParameterDropDown;
class OperatorParameterSlider;

class OpLine : public Operator
{
    Q_OBJECT
public:
    OpLine(Process *parent);
    OpLine *newInstance();
    OperatorWorker *newWorker();
    bool isBeta() const { return true; }

    typedef enum {
        Horizontal = 0,
        Vertical = 1,
    } DirectionType;
private slots:
    void selectColor(int v);
    void selectKeepBackground(int v);
    void selectDirection(int v);

private:
    OperatorParameterDropDown *m_color;
    quantum_t m_colorValue;
    OperatorParameterSlider *m_diameter;
    OperatorParameterDropDown *m_keepBackground;
    bool m_keepBackgroundValue;
    OperatorParameterDropDown *m_direction;
    DirectionType m_directionValue;
    OperatorParameterSlider *m_period;
    OperatorParameterSlider *m_gap;
    OperatorParameterSlider *m_decay;
    OperatorParameterSlider *m_skip;
};

#endif // OPLINE_H
