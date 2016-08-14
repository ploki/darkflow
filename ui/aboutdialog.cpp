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
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "darkflow.h"
#include "ports.h"
#include "darkflow-version.iss"
#include "preferences.h"

#include <Magick++.h>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    m_timer()
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    setWindowFlags(Qt::Tool);
    ui->pixmapWidget->setPixmap(QPixmap(DF_ICON));
    ui->version->setText(tr("Dark Flow version %0 %1").arg(Version).arg(DF_ARCH));
    m_timer.setInterval(500);
    connect(&m_timer,SIGNAL(timeout()), this, SLOT(updateUsage()));
    m_timer.start();
    adjustSize();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::updateUsage()
{
    if ( !isVisible() ) return;
    MagickCore::MagickSizeType area = MagickCore::GetMagickResource(MagickCore::AreaResource);
    MagickCore::MagickSizeType memory = MagickCore::GetMagickResource(MagickCore::MemoryResource);
    MagickCore::MagickSizeType map = MagickCore::GetMagickResource(MagickCore::MapResource);
    MagickCore::MagickSizeType disk = MagickCore::GetMagickResource(MagickCore::DiskResource);
    ui->lineArea->setText(tr("%0 GB").arg(double(area)/(1<<30), 0, 'f', 3));
    ui->lineMemory->setText(tr("%0 GB").arg(double(memory)/(1<<30), 0, 'f', 3));
    ui->lineMap->setText(tr("%0 GB").arg(double(map)/(1<<30), 0, 'f', 3));
    ui->lineDisk->setText(tr("%0 GB").arg(double(disk)/(1<<30), 0, 'f', 3));
    ui->lineWorkers->setText(tr("%0").arg(preferences->getAtWork()));
}
