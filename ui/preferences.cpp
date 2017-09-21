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
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QAbstractButton>
#include <QFileDialog>
#include <QSemaphore>
#include <QMutex>
#include <QMutexLocker>

#include "preferences.h"
#include "ui_preferences.h"
#include "console.h"
#include "operatorworker.h"
#include "darkflow.h"
#include "mainwindow.h"
#include <Magick++.h>

#ifdef DFL_USE_GCD
#include <thread>
#endif
QPalette dflOriginalPalette;

#ifndef QuantumRange
namespace Magick {
class ResourceLimits {
public:
    static void area(long) {}
    static long area() { return 0; }
    static void memory(long) {}
    static long memory() { return 0; }
    static void map(long) {}
    static long map() { return 0; }
    static void disk(long) {}
    static long disk() { return 0; }
    static void thread(long) {}
    static long thread() { return 0; }
};
}

#endif

static int dfl_max_threads() {
#ifdef DFL_USE_GCD
    return std::thread::hardware_concurrency();
#else
    int n = 0;
#pragma omp parallel reduction(+:n)
    n += 1;
    return n;
#endif
}
#define N_WORKERS 4
#define LAB_SEL_SIZE 256

Preferences *preferences = NULL;

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences),
  m_defaultArea(0),
  m_defaultMemory(0),
  m_defaultMap(0),
  m_defaultDisk(
      #if defined(ANDROID)
        10
      #else
        0
      #endif
      ),
  m_defaultThreads(0),
  m_sem(new QSemaphore(N_WORKERS)),
  m_mutex(new QMutex),
  m_currentMaxWorkers(N_WORKERS),
  m_scheduledMaxWorkers(1),
  m_OpenMPThreads(dfl_max_threads()),
  m_currentTarget(sRGB),
  m_incompatibleAction(Error),
  m_labSelectionSize(LAB_SEL_SIZE),
  m_palette(),
  m_atWork(0)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    setWindowFlags(Qt::Tool);
    Magick::InitializeMagick("darkflow");
    getDefaultMagickResources();

    ui->defaultDflThreads->setText(QString::number(m_OpenMPThreads));
    ui->defaultDflWorkers->setText(QString::number(m_scheduledMaxWorkers));

    bool loaded = load(false);

    if ( !loaded ) {
        getMagickResources();
        //tune ImageMagick architecture limits for first time launch
        ui->valueMemory->setText("0");
        ui->valueArea->setText("1");
        ui->valueDisk->setText(ui->valueMap->text());
#if !defined(__LP64__) && !defined(WIN64)
        ui->valueMap->setText("0");
#endif
        ui->valueDflThreads->setText(QString::number(m_OpenMPThreads));
        ui->valueDflWorkers->setText(QString::number(m_scheduledMaxWorkers));

        ui->valueTmpDir->setText(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
        ui->valueBaseDir->setText(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

        ui->comboLogLevel->setCurrentIndex(Console::Info);
        ui->comboRaiseOn->setCurrentIndex(Console::Error);
        ui->comboTrapOn->setCurrentIndex(Console::LastLevel);
        Console::setLevel(Console::Info);
        Console::setRaiseLevel(Console::Error);
        Console::setTrapLevel(Console::LastLevel);
        MagickCore::ExceptionInfo *exception = MagickCore::AcquireExceptionInfo();
        MagickCore::SetImageRegistry(MagickCore::StringRegistryType, "temporary-path", (const void*)ui->valueTmpDir->text().toLocal8Bit(), exception);

        ui->comboTransformTarget->setCurrentIndex(m_currentTarget);
        ui->spinLabSelectionSize->setValue(m_labSelectionSize);
    }
    load();

    try {
        //try to create an image!
        Magick::Image image(Magick::Geometry(1000, 1000), Magick::Color(0, 0, 0));
    } catch (std::exception &e) {
       QString msg = e.what();
       msg += "\n";
       msg += getenv("MAGICK_CONFIGURE_PATH");
       QMessageBox::warning(this, "Preliminary ImageMagick Check failed",
                            msg);
    }
}

Preferences::~Preferences()
{
    delete m_mutex;
    delete m_sem;
    delete ui;
}

QString Preferences::baseDir()
{
    return ui->valueBaseDir->text();
}

bool Preferences::acquireWorker(OperatorWorker *worker)
{
    bool success;
    do {
        int toClaim = 1;
        int currentMaxWorkers = m_currentMaxWorkers;
        {
            QMutexLocker lock(m_mutex);
            if ( m_currentMaxWorkers > m_scheduledMaxWorkers ) {
                toClaim += m_currentMaxWorkers - m_scheduledMaxWorkers;
                m_currentMaxWorkers = m_scheduledMaxWorkers;
            }
        }
        success = m_sem->tryAcquire(toClaim, 50);
        if ( !success) {
            QMutexLocker lock(m_mutex);
            m_currentMaxWorkers = currentMaxWorkers;
            if ( worker->aborted())
                break;
        }
    }
    while (!success);
    return success;
}

void Preferences::releaseWorker()
{
    int toRelease = 1;
    {
        QMutexLocker lock(m_mutex);
        if ( m_currentMaxWorkers < m_scheduledMaxWorkers ) {
            toRelease += m_scheduledMaxWorkers - m_currentMaxWorkers;
            m_currentMaxWorkers = m_scheduledMaxWorkers;
        }
    }
    m_sem->release(toRelease);
}

void Preferences::getDefaultMagickResources()
{
    const u_int64_t div = 1<<30;
    m_defaultArea    = Magick::ResourceLimits::area();
    m_defaultMemory  = Magick::ResourceLimits::memory();
    m_defaultMap     = Magick::ResourceLimits::map();
    m_defaultDisk    = Magick::ResourceLimits::disk();
    m_defaultThreads = Magick::ResourceLimits::thread();
#if defined(ANDROID)
    if ( m_defaultDisk == 0 ) {
        m_defaultDisk = 10LL * 1LL<<30;
    }
#endif
    ui->defaultArea->setText(QString::number(qreal(m_defaultArea)/div));
    ui->defaultMemory->setText(QString::number(qreal(m_defaultMemory)/div));
    ui->defaultMap->setText(QString::number(qreal(m_defaultMap)/div));
    ui->defaultDisk->setText(m_defaultDisk==(u_int64_t)-1?tr("unlimited"):QString::number(qreal(m_defaultDisk)/div));
    ui->defaultThreads->setText(QString::number(m_defaultThreads));
}

void Preferences::getMagickResources()
{
    const u_int64_t div = 1<<30;
    u_int64_t currentArea    = Magick::ResourceLimits::area();
    u_int64_t currentMemory  = Magick::ResourceLimits::memory();
    u_int64_t currentMap     = Magick::ResourceLimits::map();
    u_int64_t currentDisk    = Magick::ResourceLimits::disk();
    u_int64_t currentThreads = Magick::ResourceLimits::thread();
    ui->valueArea->setText(QString::number(qreal(currentArea)/div));
    ui->valueMemory->setText(QString::number(qreal(currentMemory)/div));
    ui->valueMap->setText(QString::number(qreal(currentMap)/div));
    ui->valueDisk->setText(currentDisk==(u_int64_t)-1?tr("unlimited"):QString::number(qreal(currentDisk)/div));
    ui->valueThreads->setText(QString::number(currentThreads));
}

void Preferences::setMagickResources()
{
    const u_int64_t div = 1<<30;
    u_int64_t currentArea    = ui->valueArea->text().toDouble()*div;
    u_int64_t currentMemory  = ui->valueMemory->text().toDouble()*div;
    u_int64_t currentMap     = ui->valueMap->text().toDouble()*div;
    u_int64_t currentDisk    = ui->valueDisk->text().toDouble()*div;
    u_int64_t currentThreads = ui->valueThreads->text().toDouble();

#if defined(ANDROID)
    if (currentDisk == 0)
        currentDisk = 10LL * 1LL<<30;
#endif
    Magick::ResourceLimits::area(currentArea);
    Magick::ResourceLimits::memory(currentMemory);
    Magick::ResourceLimits::map(currentMap);
    Magick::ResourceLimits::disk(currentDisk);
    Magick::ResourceLimits::thread(currentThreads);
    getMagickResources();
}

bool Preferences::load(bool create)
{
    const qreal mul = 1./(1<<30);
    QString filename = getAppConfigLocation() + "/config.json";
    QFile file(filename);
    if ( !file.open(QIODevice::ReadOnly)) {
        if (create) {
            dflWarning(tr("Configuration file doesn't exist, creating one"));
            save();
        }
        return false;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject obj = doc.object();
    QJsonObject resources = obj["resources"].toObject();
    QJsonObject path = obj["path"].toObject();
    QJsonObject logging = obj["logging"].toObject();
    QJsonObject pixels = obj["pixels"].toObject();
    QJsonObject style = obj["style"].toObject();

    int dflWorkers = resources["darkflowWorkers"].toInt();
    int dflThreads = resources["darkflowThreads"].toInt();
    if ( dflWorkers < 1 )
        dflWorkers = 1;
    if ( dflThreads < 1)
        dflThreads = 1;
    if ( dflThreads > 1024 )
        dflThreads = 1024;
    m_OpenMPThreads = dflThreads;

    {
        QMutexLocker lock(m_mutex);
        m_scheduledMaxWorkers = dflWorkers;
    }
    ui->valueDflThreads->setText(QString::number(m_OpenMPThreads));
    ui->valueDflWorkers->setText(QString::number(dflWorkers));

    int64_t area = resources["area"].toDouble();
    int64_t memory = resources["memory"].toDouble();
    int64_t map = resources["map"].toDouble();
    int64_t disk = resources["disk"].toDouble();
    int64_t threads = resources["threads"].toDouble();
    if ( area < 0 )
        ui->valueArea->setText(tr("unlimited"));
    else
        ui->valueArea->setText(QString::number(mul*area));
    if ( memory < 0 )
        ui->valueMemory->setText(tr("unlimited"));
    else
        ui->valueMemory->setText(QString::number(mul*memory));
    if ( map < 0 )
        ui->valueMap->setText(tr("unlimited"));
    else
        ui->valueMap->setText(QString::number(mul*map));
    if ( disk < 0 )
        ui->valueDisk->setText(tr("unlimited"));
    else
        ui->valueDisk->setText(QString::number(mul*disk));
    if ( threads < 1 )
        ui->valueThreads->setText(tr("unlimited"));
    else
        ui->valueThreads->setText(QString::number(threads));

    setMagickResources();

    m_currentTarget = TransformTarget(pixels["transformTarget"].toInt());
    ui->comboTransformTarget->setCurrentIndex(m_currentTarget);
    m_incompatibleAction = IncompatibleAction(pixels["incompatibleAction"].toInt());
    ui->comboIncompatibleScale->setCurrentIndex(m_incompatibleAction);
    m_labSelectionSize = pixels["labSelectionSize"].toInt();
    if ( 0 == m_labSelectionSize )
        m_labSelectionSize = LAB_SEL_SIZE;
    ui->spinLabSelectionSize->setValue(m_labSelectionSize);

    ui->valueTmpDir->setText(path["tmp"].toString());
    ui->valueBaseDir->setText(path["base"].toString());
    QDir().mkpath(path["tmp"].toString());
    QDir().mkpath(path["base"].toString());

    MagickCore::ExceptionInfo *exception = MagickCore::AcquireExceptionInfo();
    MagickCore::SetImageRegistry(MagickCore::StringRegistryType, "temporary-path", (const void*)ui->valueTmpDir->text().toLocal8Bit(), exception);
    dflDebug("magick tmp = %s", (const char*)MagickCore::GetImageRegistry(MagickCore::StringRegistryType, "temporary-path", exception));
    ui->comboLogLevel->setCurrentIndex(logging["level"].toInt());
    ui->comboRaiseOn->setCurrentIndex(logging["raise"].toInt());
    ui->comboTrapOn->setCurrentIndex(logging["trap"].toInt());

    Console::setLevel(Console::Level(logging["level"].toInt()));
    Console::setRaiseLevel(Console::Level(logging["raise"].toInt()));
    Console::setTrapLevel(Console::Level(logging["trap"].toInt()));

    loadStyle(style);

    return true;
}



void Preferences::save()
{
    QJsonObject resources;
    QJsonObject pixels;
    QJsonObject path;
    QJsonObject logging;
    QJsonObject style = saveStyle();

    QString areaStr = ui->valueArea->text();
    QString memoryStr = ui->valueMemory->text();
    QString mapStr = ui->valueMap->text();
    QString diskStr = ui->valueDisk->text();
    QString threadsStr = ui->valueThreads->text();

    const u_int64_t mul = 1<<30;

    resources["area"] =  qint64((areaStr.isEmpty()||areaStr==tr("unlimited"))?-1:areaStr.toDouble()*mul);
    resources["memory"] = qint64((memoryStr.isEmpty()||memoryStr==tr("unlimited"))?-1:memoryStr.toDouble()*mul);
    resources["map"] = qint64((mapStr.isEmpty()||mapStr==tr("unlimited"))?-1:mapStr.toDouble()*mul);
    resources["disk"] = qint64((diskStr.isEmpty()||diskStr==tr("unlimited"))?-1:diskStr.toDouble()*mul);
    resources["threads"] = qint64((threadsStr.isEmpty()||threadsStr==tr("unlimited"))?-1:threadsStr.toLongLong());

    qint64 dflThreads = ui->valueDflThreads->text().toLong();
    qint64 dflWorkers = ui->valueDflWorkers->text().toLong();
    if ( dflThreads < 1 )
        dflThreads = 1;
    if ( dflWorkers < 1 )
        dflWorkers = 1;
    resources["darkflowWorkers"] = dflWorkers;
    resources["darkflowThreads"] = dflThreads;

    m_currentTarget = TransformTarget(ui->comboTransformTarget->currentIndex());
    pixels["transformTarget"] = m_currentTarget;
    m_incompatibleAction = IncompatibleAction(ui->comboIncompatibleScale->currentIndex());
    pixels["incompatibleAction"] = m_incompatibleAction;
    m_labSelectionSize = ui->spinLabSelectionSize->value();
    pixels["labSelectionSize"] = m_labSelectionSize;

    path["tmp"] = ui->valueTmpDir->text();
    path["base"] = ui->valueBaseDir->text();

    logging["level"] = ui->comboLogLevel->currentIndex();
    logging["raise"] = ui->comboRaiseOn->currentIndex();
    logging["trap"] = ui->comboTrapOn->currentIndex();

    QJsonObject obj;
    obj["resources"] = resources;
    obj["pixels"] = pixels;
    obj["path"] = path;
    obj["logging"] = logging;
    obj["style"] = style;
    QJsonDocument doc;
    doc.setObject(obj);

    QString filename = getAppConfigLocation() + "/config.json";
    QDir dir(getAppConfigLocation());
    if ( !dir.exists() ) {
        if ( !dir.mkpath(getAppConfigLocation()) ) {
            dflCritical("Unable to create configuration directory");
            return;
        }
    }
    QFile saveFile(filename);
    if ( !saveFile.open(QIODevice::WriteOnly) ) {
        dflCritical(tr("Could not save configuration file %0").arg(filename));
        return;
    }
    saveFile.write(doc.toJson());
}

QJsonObject Preferences::saveStyle()
{
    QJsonObject obj;
    QString alternateBase = ui->lineAlternateBase->text();
    QString base = ui->lineBase->text();
    QString text = ui->lineText->text();
    QString window = ui->lineWindow->text();
    QString windowText = ui->lineWindowText->text();
    QString button = ui->lineButton->text();
    QString buttonText = ui->lineButtonText->text();
    QString highlight = ui->lineHighlight->text();
    QString highlightedText = ui->lineHighlightedText->text();
    QString toolTipBase = ui->lineToolTipBase->text();
    QString toolTipText = ui->lineToolTipText->text();
    QString link = ui->lineLink->text();
    QString brightText = ui->lineBrightText->text();
    bool onlyWorkspace = ui->checkBoxOnlyWorkspace->isChecked();
    QString fontFamily = ui->valueFontFamily->currentText();
    int fontSize = ui->valueFontSize->value();

    obj["alternateBase"] = alternateBase;
    obj["base"] = base;
    obj["text"] = text;
    obj["window"] = window;
    obj["windowText"] = windowText;
    obj["button"] = button;
    obj["buttonText"] = buttonText;
    obj["highlight"] = highlight;
    obj["highlightedText"] = highlightedText;
    obj["toolTipBase"] = toolTipBase;
    obj["toolTipText"] = toolTipText;
    obj["link"] = link;
    obj["brightText"] = brightText;
    obj["onlyWorkspace"] = onlyWorkspace;
    obj["workspaceFontFamily"] = fontFamily;
    obj["workspaceFontSize"] = fontSize;
    return obj;
}

void Preferences::loadStyle(QJsonObject &obj)
{
    if ( obj.isEmpty() )
        obj = saveStyle();

    QString alternateBase = obj["alternateBase"].toString();
    QString base = obj["base"].toString();
    QString text = obj["text"].toString();
    QString window = obj["window"].toString();
    QString windowText = obj["windowText"].toString();
    QString button = obj["button"].toString();
    QString buttonText = obj["buttonText"].toString();
    QString highlight = obj["highlight"].toString();
    QString highlightedText = obj["highlightedText"].toString();
    QString toolTipBase = obj["toolTipBase"].toString();
    QString toolTipText = obj["toolTipText"].toString();
    QString link = obj["link"].toString();
    QString brightText = obj["brightText"].toString();
    bool onlyWorkspace = obj["onlyWorkspace"].toBool();
    QString fontFamily = obj["workspaceFontFamily"].toString();
    int fontSize = obj["workspaceFontSize"].toInt();
    if (fontSize < 1) {
#if defined(ANDROID)
        fontSize = 7;
#else
        fontSize = 12;
#endif
    }

    ui->lineAlternateBase->setText(alternateBase);
    ui->lineBase->setText(base);
    ui->lineText->setText(text);
    ui->lineWindow->setText(window);
    ui->lineWindowText->setText(windowText);
    ui->lineButton->setText(button);
    ui->lineButtonText->setText(buttonText);
    ui->lineHighlight->setText(highlight);
    ui->lineHighlightedText->setText(highlightedText);
    ui->lineToolTipBase->setText(toolTipBase);
    ui->lineToolTipText->setText(toolTipText);
    ui->lineLink->setText(link);
    ui->lineBrightText->setText(brightText);
    ui->checkBoxOnlyWorkspace->setChecked(onlyWorkspace);
    ui->valueFontFamily->setCurrentFont(QFont(fontFamily));
    ui->valueFontSize->setValue(fontSize);

    QPalette palette;
    palette.setColor(QPalette::AlternateBase, alternateBase);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, windowText);
    palette.setColor(QPalette::Button, button);
    palette.setColor(QPalette::ButtonText, buttonText);
    palette.setColor(QPalette::Highlight, highlight);
    palette.setColor(QPalette::HighlightedText, highlightedText);
    palette.setColor(QPalette::ToolTipBase, toolTipBase);
    palette.setColor(QPalette::ToolTipText, toolTipText);
    palette.setColor(QPalette::Link, link);
    palette.setColor(QPalette::BrightText, brightText);
    m_palette = palette;

    if ( !onlyWorkspace)
        qApp->setPalette(palette);
    else
        qApp->setPalette(dflOriginalPalette);

    if (dflMainWindow)
        dflMainWindow->setSceneBackgroundBrush(alternateBase);

}

void Preferences::clicked(QAbstractButton *button)
{
    bool doClose = false;
    bool doSave = false;
    switch (ui->buttonBox->buttonRole(button))
    {
    case QDialogButtonBox::ApplyRole:
        doSave = true;
        break;
    case QDialogButtonBox::ResetRole:
        break;
    case QDialogButtonBox::AcceptRole:
        doSave = true;
        doClose = true;
        break;
    case QDialogButtonBox::RejectRole:
    default:
        doClose = true;
        break;
    }
    if (doSave)
        save();
    load();
    if (doClose)
        hide();
}

void Preferences::accept()
{
    save();
    load();
    hide();
}

void Preferences::reject()
{
    load();
    hide();
}

void Preferences::tmpDirClicked()
{
    QString tmpDir = QFileDialog::getExistingDirectory(this, tr("Temporary Directory"),
                                                          ui->valueTmpDir->text(),
                                                          QFileDialog::ShowDirsOnly);
    if ( !tmpDir.isEmpty())
        ui->valueTmpDir->setText(tmpDir);
}

void Preferences::baseDirClicked()
{
    QString baseDir = QFileDialog::getExistingDirectory(this, tr("Base Directory"),
                                                          ui->valueBaseDir->text(),
                                                          QFileDialog::ShowDirsOnly);
    if ( !baseDir.isEmpty())
        ui->valueBaseDir->setText(baseDir);
}

Preferences::TransformTarget Preferences::getCurrentTarget() const
{
    return m_currentTarget;
}

Preferences::IncompatibleAction Preferences::getIncompatibleAction() const
{
    return m_incompatibleAction;
}

int Preferences::getNumThreads() const
{
    return m_OpenMPThreads;
}

int Preferences::getMagickNumThreads() const
{
    return Magick::ResourceLimits::thread();
}

int Preferences::getLabSelectionSize() const
{
    return m_labSelectionSize;
}

QString Preferences::getAppConfigLocation()
{
    QString writableDirectory;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    writableDirectory = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
#else
    writableDirectory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + "/" + DF_APPNAME;
#endif
   QDir().mkpath(writableDirectory);
   return writableDirectory;
}

QColor Preferences::color(QPalette::ColorRole role)
{
    return m_palette.color(role);
}

void Preferences::incrAtWork()
{
    atomic_incr(&m_atWork);
}

void Preferences::decrAtWork()
{
    atomic_decr(&m_atWork);
}

unsigned long Preferences::getAtWork()
{
    return m_atWork;
}

QFont Preferences::getWorkspaceFont() const
{
    QFont font = ui->valueFontFamily->currentFont();
    int size = ui->valueFontSize->value();
    if (size > 0)
        font.setPointSize(size);
    return font;
}

QFont Preferences::getWorkspaceFontFamily() const
{
    return ui->valueFontFamily->currentFont();
}
