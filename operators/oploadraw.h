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
#ifndef OPERATORLOADRAW_H
#define OPERATORLOADRAW_H

#include "operator.h"
#include <QObject>

class Process;
class OperatorParameterFilesCollection;
class OperatorParameterDropDown;
class OpLoadRaw : public Operator
{
    Q_OBJECT
public:
    OpLoadRaw(Process *parent);
    ~OpLoadRaw();
    OpLoadRaw *newInstance();

    typedef enum {
        Linear,
        sRGB,
        IUT_BT_709
    } ColorSpace;
    typedef enum {
        NoDebayer,
        HalfSize,
        Low,
        VNG,
        PPG,
        AHD,
    } Debayer;
    typedef enum {
        NoWhiteBalance,
        RawColors,
        Camera,
        Daylight,
    } WhiteBalance;
    typedef enum {
        ClipAuto,
        Clip16bit,
        Clip15bit,
        Clip14bit,
        Clip13bit,
        Clip12bit
    } Clipping;
    QStringList getCollection() const;

    QString getColorSpace() const;
    QString getDebayer() const;
    QString getWhiteBalance() const;

public slots:
    void setColorSpace(int v);
    void setDebayer(int v);
    void setWhiteBalance(int v);
    void setClipping(int v);

    void filesCollectionChanged();

    OperatorWorker *newWorker();

protected:


private:
    friend class WorkerLoadRaw;
    OperatorParameterFilesCollection *m_filesCollection;
    OperatorParameterDropDown *m_colorSpace;
    OperatorParameterDropDown *m_debayer;
    OperatorParameterDropDown *m_whiteBalance;
    OperatorParameterDropDown *m_clipping;

    ColorSpace m_colorSpaceValue;
    Debayer m_debayerValue;
    WhiteBalance m_whiteBalanceValue;
    Clipping m_clippingValue;

};

#endif // OPERATORLOADRAW_H
