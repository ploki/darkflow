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
#include "oploadvideo.h"
#include "workerloadvideo.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterslider.h"
#include "operatoroutput.h"
#include "process.h"

OpLoadVideo::OpLoadVideo(Process *process) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Videos"), Operator::NA, process),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "videoCollection",
                          tr("Videos"),
                          tr("Select Videos to add to the collection"),
                          m_process->baseDirectory(),
                          tr("Videos (*.avi *.mjpg *.mjpeg *.ts *.m2ts *.mov *.mpg *.mpeg *.mp4 *.webm);;"
                          "All Files (*.*)"), this)),
    m_skip(new OperatorParameterSlider("skip", tr("Skip"), tr("Video Frames to skip"), Slider::Value, Slider::Linear, Slider::Integer, 0, 1000, 0, 0, 1000000, Slider::FilterPixels, this)),
    m_count(new OperatorParameterSlider("count", tr("Count"), tr("Video Frames to collect"), Slider::Value, Slider::Linear, Slider::Integer, 0, 1000, 100, 0, 1000000, Slider::FilterPixels, this))
{
    addParameter(m_filesCollection);
    addParameter(m_skip);
    addParameter(m_count);
    addOutput(new OperatorOutput(tr("Video frames"), this));
}

OpLoadVideo::~OpLoadVideo()
{
}

OpLoadVideo *OpLoadVideo::newInstance()
{
    return new OpLoadVideo(m_process);
}

OperatorWorker *OpLoadVideo::newWorker()
{
    return new WorkerLoadVideo(m_thread, this);
}

QStringList OpLoadVideo::getCollection() const
{
    return m_filesCollection->collection();
}

int OpLoadVideo::getSkip() const
{
    return DF_ROUND(m_skip->value());
}

int OpLoadVideo::getCount() const
{
    return DF_ROUND(m_count->value());
}

void OpLoadVideo::filesCollectionChanged()
{
    setOutOfDate();
}
