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
#ifndef SELECTIVELAB_H
#define SELECTIVELAB_H

#include <QDialog>
#include <QRunnable>
#include <QMutex>

namespace Ui {
class SelectiveLab;
}

class Photo;
class Operator;
class QPixmap;

class SelectiveLab : public QDialog, public QRunnable
{
    Q_OBJECT

public:
    explicit SelectiveLab(const QString& windowCaption,
                          int hue,
                          int coverage,
                          bool strict,
                          int level,
                          bool clipToGamut,
                          bool displayGuide,
                          bool preview,
                          const Operator *op,
                          QWidget *parent);
    ~SelectiveLab();

    int hue() const;
    void setHue(int v);

    int coverage() const;
    void setCoverage(int v);

    bool strict() const;
    void setStrict(bool v);

    int level() const;
    void setLevel(int v);

    bool clipToGamut() const;
    void setClipToGamut(bool clipToGamut);

    bool displayGuide() const;
    void setDisplayGuide(bool v);

    bool previewEffect() const;
    void setPreviewEffect(bool v);

    void loadValues(int hue,
                    int coverage,
                    bool strict,
                    int level,
                    bool clipToValue,
                    bool displayGuide,
                    bool preview);
    void getDialogValues();
    void run();

public slots:
    void updateView();
    void updateViewNoEmission();
    void updatePixmap();

signals:
    void updated();
    void finishedPixmap();

protected:
    Photo createPhoto(int level, bool clipToGamut);
    void applyPreview(Photo& photo);
    void drawGuide(Photo& photo, int hue, int coverage, bool strict);

private:
    Ui::SelectiveLab *ui;
    const Operator *m_operator;

    int m_labSelectionSize;
    bool m_updated;
    int m_hue;
    int m_coverage;
    int m_level;
    bool m_strict;
    bool m_clipToGamut;
    bool m_guide;
    bool m_preview;
    bool m_running;
    QPixmap m_pixmap;
    QMutex m_mutex;

};

#endif // SELECTIVELAB_H
