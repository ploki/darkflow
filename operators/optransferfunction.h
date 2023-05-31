/*
 * Copyright (c) 2006-2021, Guillaume Gimenez <guillaume@blackmilk.fr>
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
#ifndef OPTRANSFERFUNCTION_H
#define OPTRANSFERFUNCTION_H

#include "operator.h"

class OperatorParameterSlider;
class OperatorParameterDropDown;
class WorkerTransferFunction;

class OpTransferFunction : public Operator
{
    Q_OBJECT
public:
    OpTransferFunction(Process *parent);

    OpTransferFunction *newInstance();
    OperatorWorker *newWorker();

public slots:
    void selectEncoding(int v);
    void selectScale(int v);
    void selectGrid(int v);
private:
    friend WorkerTransferFunction;
    typedef enum {
        Identity,
        Linear,
        Gamma,
        Log
    } Encoding;
    OperatorParameterSlider *m_widthDialog;
    OperatorParameterDropDown *m_gridDialog;
    bool m_grid;
    OperatorParameterDropDown *m_encodingDialog;
    Encoding m_encoding;
    OperatorParameterDropDown *m_scaleDialog;
    Encoding m_scale;
};

#endif // OPTRANSFERFUNCTION_H
