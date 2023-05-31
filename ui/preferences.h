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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ports.h"
#include <QDialog>

namespace Ui {
class Preferences;
}
class QAbstractButton;
class QSemaphore;
class QMutex;
class OperatorWorker;
class QFont;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    typedef enum {
        Linear,
        sRGB,
        IUT_BT_709,
        SquareRoot,
        TargetCustom,
    } TransformTarget;
    typedef enum {
        Ignore,
        Warning,
        Error
    } IncompatibleAction;
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

    QString baseDir();
    QString tmpDir();

    bool acquireWorker(OperatorWorker *worker);
    void releaseWorker();

    TransformTarget getCurrentRenderTarget() const;
    TransformTarget getCurrentDisplayTarget() const;
    qreal getCurrentCustomGamma() const;
    qreal getCurrentCustomRange() const;
    IncompatibleAction getIncompatibleAction() const;
    int getNumThreads() const;
    int getMagickNumThreads() const;
    int getLabSelectionSize() const;
    static QString getAppConfigLocation();
    QColor color(QPalette::ColorRole role);
    void incrAtWork();
    void decrAtWork();
    unsigned long getAtWork();
    QFont getWorkspaceFont() const;
    QFont getWorkspaceFontFamily() const;

private:
    void getDefaultMagickResources();
    void getMagickResources();
    void setMagickResources();
    bool load(bool create=true);
    void save();
    QJsonObject saveStyle();
    void loadStyle(QJsonObject& obj);

public slots:
    void clicked(QAbstractButton * button);
    void accept();
    void reject();
    void tmpDirClicked();
    void baseDirClicked();

private:
    Ui::Preferences *ui;
    u_int64_t m_defaultArea;
    u_int64_t m_defaultMemory;
    u_int64_t m_defaultMap;
    u_int64_t m_defaultDisk;
    u_int64_t m_defaultThreads;
    QSemaphore *m_sem;
    QMutex *m_mutex;
    u_int64_t m_currentMaxWorkers;
    u_int64_t m_scheduledMaxWorkers;
    u_int64_t m_OpenMPThreads;
    TransformTarget m_currentRenderTarget;
    TransformTarget m_currentDisplayTarget;
    qreal m_currentCustomGamma;
    qreal m_currentCustomRange;
    IncompatibleAction m_incompatibleAction;
    int m_labSelectionSize;
    QPalette m_palette;
    unsigned long m_atWork;
};

extern Preferences *preferences;
extern QPalette dflOriginalPalette;

#endif // PREFERENCES_H
