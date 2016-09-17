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
#include "opsave.h"
#include "operatorparameterdirectory.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatorworker.h"
#include "process.h"
#include "photo.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>

class WorkerSave : public OperatorWorker {
    QString m_targetDirectory;
    OpSave::SaveType m_fileType;
    bool m_backup;
public:
    WorkerSave(const QString &targetDirectory,
               OpSave::SaveType fileType,
               bool backup,
               QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_targetDirectory(targetDirectory),
        m_fileType(fileType),
        m_backup(backup)
    {
    }
    Photo process(const Photo&, int, int) { throw 0; }
    void play() {
        for (int i = 0, s = m_inputs[0].count() ; i < s ; ++i ) {
            Photo& photo = m_inputs[0][i];
            QDir targetDir(m_targetDirectory);
            QString magick;
            QString baseFilename = targetDir.filePath(photo.getTag(TAG_NAME));
            QString ext;
            switch (m_fileType) {
            case OpSave::SaveFITS:
                ext = ".fits"; magick = "FITS"; break;
            case OpSave::SaveTIFF:
                ext = ".tif"; magick = "TIFF"; break;
            case OpSave::SaveJPEG:
                ext = ".jpg"; magick = "JPG"; break;
            }
            QFileInfo finfo(baseFilename + ext);
            if ( m_backup && finfo.exists() ) {
                QDateTime lastModified = finfo.lastModified();
                QString ts = lastModified.toString("yyyy'-'dd'-'MM'T'hh':'mm':'ss'.'zzz'Z'");
                QFile orig(baseFilename + ext);
                orig.rename(baseFilename + "-" + ts + ext);
            }
            QString filename = finfo.filePath();
            photo.save(filename, magick);
            emitProgress(i, s, 0, 1);
        }
        emitSuccess();

    }
};

static const char *SaveTypeStr[] = {
    QT_TRANSLATE_NOOP("OpSave", "FITS"),
    QT_TRANSLATE_NOOP("OpSave", "TIFF"),
    QT_TRANSLATE_NOOP("OpSave", "JPEG"),
};

OpSave::OpSave(Process *parent) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Save"), Operator::All, parent),
    m_directory(new OperatorParameterDirectory("directory", tr("Target directory"), m_process->baseDirectory(), "", this)),
    m_fileType(new OperatorParameterDropDown("fileType", tr("File type"), this, SLOT(selectType(int)))),
    m_fileTypeValue(SaveFITS),
    m_backup(new OperatorParameterDropDown("backup", tr("Backup"), this, SLOT(selectBackup(int))))
{
    m_fileType->addOption(DF_TR_AND_C(SaveTypeStr[SaveFITS]), SaveFITS, true);
    m_fileType->addOption(DF_TR_AND_C(SaveTypeStr[SaveTIFF]), SaveTIFF);
    m_fileType->addOption(DF_TR_AND_C(SaveTypeStr[SaveJPEG]), SaveJPEG);

    m_backup->addOption(DF_TR_AND_C("No"), false, true);
    m_backup->addOption(DF_TR_AND_C("Yes"), true);

    addParameter(m_directory);
    addParameter(m_fileType);
    addParameter(m_backup);
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
}

OpSave *OpSave::newInstance()
{
    return new OpSave(m_process);
}

OperatorWorker *OpSave::newWorker()
{
    return new WorkerSave(m_directory->currentValue(), m_fileTypeValue, m_backupValue, m_thread, this);
}

void OpSave::selectType(int v)
{
    if ( m_fileTypeValue != v ) {
        m_fileTypeValue = SaveType(v);
        setOutOfDate();
    }
}

void OpSave::selectBackup(int v)
{
    if ( m_backupValue != !!v ) {
        m_backupValue = !!v;
        //no need to invalidate process
    }
}
