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
#include <QThreadPool>
#include <QMutexLocker>

#include "selectivelab.h"
#include "ui_selectivelab.h"
#include "preferences.h"
#include "console.h"
#include "photo.h"
#include "igamma.h"
#include <Magick++.h>
#include "cielab.h"
#include "darkflow.h"
#include "algorithm.h"
#include "operator.h"

using Magick::Quantum;

SelectiveLab::SelectiveLab(const QString& windowCaption,
                           int hue,
                           int coverage,
                           bool strict,
                           int level,
                           bool clipToGamut,
                           bool displayGuide,
                           bool preview,
                           const Operator *op,
                           QWidget *parent) :
    QDialog(parent),
    QRunnable(),
    ui(new Ui::SelectiveLab),
    m_operator(op),
    m_labSelectionSize(preferences->getLabSelectionSize()),
    m_updated(true),
    m_hue(hue),
    m_coverage(coverage),
    m_level(level),
    m_strict(strict),
    m_clipToGamut(clipToGamut),
    m_guide(displayGuide),
    m_preview(preview),
    m_running(false),
    m_pixmap(),
    m_mutex(QMutex::Recursive)

{
    if ( m_labSelectionSize == 0 ) {
        dflError(tr("Wrong lab selection size of 0"));
        m_labSelectionSize = 32;
    }
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    setWindowFlags(Qt::Tool);
    ui->view->setMinimumSize(m_labSelectionSize, m_labSelectionSize);
    ui->view->resize(m_labSelectionSize, m_labSelectionSize);

    setWindowTitle(windowCaption);
    setHue(hue);
    setCoverage(coverage);
    setStrict(strict);
    setLevel(level);
    setDisplayGuide(displayGuide);
    setPreviewEffect(preview);
    adjustSize();
    updateView();
    connect(m_operator, SIGNAL(outOfDate()), this, SLOT(updateViewNoEmission()));
    connect(this, SIGNAL(finishedPixmap()), this, SLOT(updatePixmap()), Qt::QueuedConnection);
    getDialogValues();
    setAutoDelete(false);
}

SelectiveLab::~SelectiveLab()
{
    m_running = true;
    QThreadPool::globalInstance()->waitForDone();
    delete ui;
}

int SelectiveLab::hue() const
{
    return ui->sliderHue->value();
}

void SelectiveLab::setHue(int v)
{
    ui->sliderHue->setValue(v);
}

int SelectiveLab::coverage() const
{
    return ui->sliderCoverage->value();
}

void SelectiveLab::setCoverage(int v)
{
    ui->sliderCoverage->setValue(v);
}

bool SelectiveLab::strict() const
{
    return ui->checkBoxStrict->isChecked();
}

void SelectiveLab::setStrict(bool v)
{
    ui->checkBoxStrict->setChecked(v);
}

int SelectiveLab::level() const
{
    return ui->sliderView->value();
}

void SelectiveLab::setLevel(int v)
{
    ui->sliderView->setValue(v);
}

bool SelectiveLab::clipToGamut() const
{
    return ui->checkBoxClipToGamut->isChecked();
}

void SelectiveLab::setClipToGamut(bool clipToGamut)
{
    ui->checkBoxClipToGamut->setChecked(clipToGamut);
}

bool SelectiveLab::displayGuide() const
{
    return ui->checkBoxGuide->isChecked();
}

void SelectiveLab::setDisplayGuide(bool v)
{
    ui->checkBoxGuide->setChecked(v);
}

bool SelectiveLab::previewEffect() const
{
    return ui->checkBoxPreview->isChecked();
}

void SelectiveLab::setPreviewEffect(bool v)
{
    ui->checkBoxPreview->setChecked(v);
}

void SelectiveLab::loadValues(int hue, int coverage, bool strict, int level, bool clipToGamut, bool displayGuide, bool preview)
{
    setHue(hue);
    setCoverage(coverage);
    setStrict(strict);
    setLevel(level);
    setClipToGamut(clipToGamut);
    setDisplayGuide(displayGuide);
    setPreviewEffect(preview);
    updateView();
}

void SelectiveLab::getDialogValues()
{
    do {
        QMutexLocker lock(&m_mutex);
        m_updated = true;
        m_hue = ui->sliderHue->value();
        m_coverage = ui->sliderCoverage->value();
        m_level = ui->sliderView->value();
        m_strict = ui->checkBoxStrict->isChecked();
        m_clipToGamut = ui->checkBoxClipToGamut->isChecked();
        m_guide = ui->checkBoxGuide->isChecked();
        m_preview = ui->checkBoxPreview->isChecked();
        if ( m_hue == -181 ) {
            m_hue = 179;
            ui->sliderHue->setValue(m_hue);
            break;
        }
        if ( m_hue == 181 ) {
            m_hue = -179;
            ui->sliderHue->setValue(m_hue);
            break;
        }
    }
    while (0);
}

void SelectiveLab::run()
{
    int hue, coverage, strict;
    int level;
    bool preview, guide, clipToGamut;
    do {
        QMutexLocker lock(&m_mutex);
        hue = m_hue;
        coverage = m_coverage;
        strict = m_strict;
        level = m_level;
        preview = m_preview;
        guide = m_guide;
        clipToGamut = m_clipToGamut;
    }
    while (0);

    Photo photo = createPhoto(level, clipToGamut);

    if (preview)
        applyPreview(photo);

    if (guide)
        drawGuide(photo, hue, coverage, strict);
    bool hdr = photo.getScale() == Photo::HDR;
    STFLut stf(hdr, 1, 0, SRGB_G, SRGB_N, 1.);
    QPixmap pixmap =  photo.imageToPixmap(stf);
    do {
        QMutexLocker lock(&m_mutex);
        m_pixmap = pixmap;
    }
    while (0);

    emit finishedPixmap();
}

void SelectiveLab::updateViewNoEmission()
{
    getDialogValues();
    do {
        QMutexLocker lock(&m_mutex);
        if ( m_running )
            return;
        m_running = true;
        m_updated = false;
    }
    while (0);
    Q_ASSERT( m_running );
    QThreadPool::globalInstance()->start(this);
}

void SelectiveLab::updatePixmap()
{
    Q_ASSERT( this->thread() == QThread::currentThread() );
    do {
        m_running = false;
        QMutexLocker lock(&m_mutex);
        ui->view->setPixmap(m_pixmap);
    }
    while (0);
    if ( m_updated ) {
        updateViewNoEmission();
    }
}

void SelectiveLab::updateView()
{
    updateViewNoEmission();
    emit updated();
}

Photo SelectiveLab::createPhoto(int level, bool clipToGamut)
{
    Photo photo;
    photo.createImage(m_labSelectionSize, m_labSelectionSize);
    Magick::Image& image = photo.image();
    int w = image.columns();
    int h = image.rows();
    double v = double(level)/100.;

    std::shared_ptr<Ordinary::Pixels> pixel_cache(new Ordinary::Pixels(image));
    dfl_parallel_for(y, 0, h, 4, (image), {
        Magick::PixelPacket *pixels = pixel_cache->get(0,y,w,1);
        if ( !pixels ) continue;
        for ( int x = 0 ; x < w ; ++x ) {
            quantum_t rgb[3];
            double lab[3], check[3];

            lab[0] = 100.*v;//lab_gammaize(v);
            lab[1] = 2*DF_MAX_AB*double(x-w/2)/(double(w));
            lab[2] = -2*DF_MAX_AB*double(y-w/2)/(double(w));
            CIELab_to_RGB(lab,rgb);
            RGB_to_CIELab(rgb, check);

            if ( clipToGamut && ( !DF_EQUALS(lab[0],check[0],0.1) ||
                                  !DF_EQUALS(lab[1],check[1],0.1) ||
                                  !DF_EQUALS(lab[2],check[2],0.1))) {
                pixels[x].red=pixels[x].green=pixels[x].blue=0;
            }
            else {
                pixels[x].red=rgb[0];
                pixels[x].green=rgb[1];
                pixels[x].blue=rgb[2];
            }
        }
        pixel_cache->sync();
    });

    return photo;
}

void SelectiveLab::applyPreview(Photo &photo)
{
    Algorithm *algo = m_operator->getAlgorithm();
    if (!algo) {
        dflWarning(tr("Operator does not implement getAlgorithm()"));
        return;
    }
    algo->applyOn(photo);
    m_operator->releaseAlgorithm(algo);
}

void SelectiveLab::drawGuide(Photo &photo, int hue, int coverage, bool strict)
{
    bool inv_sat = false;
    if (coverage < 0) {
        coverage = -coverage;
        inv_sat = true;
    }
    //calcul de la puissance de largeur
    double theta = M_PI - M_PI * (double(coverage)/2.) / 180.;
    double puissance = 0;
    if ( coverage != 0 ) {
        puissance = (log(-M_LN2/(log(-(cos(theta)-1.)/2.)))+2.*M_LN2)/(M_LN2);
        puissance = pow(2.,puissance);
    }
    //calcul de l'angle d'application
    theta = M_PI * double((360+hue)%360)/180.;

    double bias_sat = strict?0:1;

    Magick::Image &image = photo.image();
    Magick::Image srcImage = Magick::Image(image);
    ResetImage(image);
    int w = srcImage.columns();
    int h = srcImage.rows();
    int s;
    Ordinary::Pixels src_cache(srcImage);
    Ordinary::Pixels dst_cache(image);
    const Magick::PixelPacket *src = src_cache.getConst(0, 0, w, h);
    Magick::PixelPacket *dst = dst_cache.get(0, 0, w, h);

    memcpy(dst, src, w*h*sizeof(*dst));

    s = m_labSelectionSize*m_labSelectionSize/4;
    dfl_parallel_for(i, 0, s, 1024, (), {
        double t = 2*M_PI*i/s;
        double mul;
        int x, y, c;

        quantum_t l;
        Magick::PixelPacket *p;

        x = clamp<int>(m_labSelectionSize/2 - cos(t)*(m_labSelectionSize/4-1),0,m_labSelectionSize);
        y = clamp<int>(m_labSelectionSize/2 - sin(t)*(m_labSelectionSize/4-1),0,m_labSelectionSize);
        l = DF_ROUND(LUMINANCE_PIXEL(src[y*w+x]));
        p = dst+y*w+x;
        p->red = p->green = p->blue = 8192;



        if ( coverage == 360 ) {
            if ( inv_sat ) {
                mul = bias_sat;
            }
            else
                mul = 2;
        }
        else {
            if ( puissance != 0 ) {
                const double epsilon = pow((1.-cos(t+theta))/2.,puissance);
                if ( inv_sat )
                    mul = bias_sat+(1.-epsilon);
                else
                    mul = bias_sat+epsilon;
            }
            else {
                mul = bias_sat;
            }
            if ( bias_sat == 0 )
                mul *= 2;
        }

        x = clamp<int>(m_labSelectionSize/2 - mul*cos(t)*(m_labSelectionSize/4-1),0,m_labSelectionSize);
        y = clamp<int>(m_labSelectionSize/2 - mul*sin(t)*(m_labSelectionSize/4-1),0,m_labSelectionSize);
        l = DF_ROUND(LUMINANCE_PIXEL(src[y*w+x]));
        p = dst+y*w+x;
        if ( l < 16384 ) c = QuantumRange; else c = 0;
        p->red = p->green = p->blue = c;
    });
    const int center = w*m_labSelectionSize/2+m_labSelectionSize/2;
    dst[center].red = dst[center].green = dst[center].blue = QuantumRange;
    dst[center-1].red = dst[center-1].green = dst[center-1].blue = QuantumRange;
    dst[center+1].red = dst[center+1].green = dst[center+1].blue = QuantumRange;
    dst[center-m_labSelectionSize].red = dst[center-m_labSelectionSize].green = dst[center-m_labSelectionSize].blue = QuantumRange;
    dst[center+m_labSelectionSize].red = dst[center+m_labSelectionSize].green = dst[center+m_labSelectionSize].blue = QuantumRange;

    dst_cache.sync();

}
