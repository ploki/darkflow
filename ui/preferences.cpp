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
#include <Magick++.h>

#include <omp.h>

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
#if 0
    return omp_get_max_threads();
#else
    int n = 0;
#pragma omp parallel reduction(+:n)
    n += 1;
    return n;
#endif
}
#define N_WORKERS 4

Preferences *preferences = NULL;

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences),
  m_defaultArea(0),
  m_defaultMemory(0),
  m_defaultMap(0),
  m_defaultDisk(0),
  m_defaultThreads(0),
  m_sem(new QSemaphore(N_WORKERS)),
  m_mutex(new QMutex),
  m_currentMaxWorkers(N_WORKERS),
  m_scheduledMaxWorkers(N_WORKERS),
  m_OpenMPThreads(dfl_max_threads()),
  m_currentTarget(sRGB),
  m_incompatibleAction(Error),
  m_labSelectionSize(256)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
    Magick::InitializeMagick("darkflow");
    getDefaultMagickResources();

    ui->defaultDflThreads->setText(QString::number(m_OpenMPThreads));
    ui->defaultDflWorkers->setText(QString::number(m_scheduledMaxWorkers));

    bool loaded = load(false);

    if ( !loaded ) {
        getMagickResources();

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
    ui->defaultArea->setText(QString::number(qreal(m_defaultArea)/div));
    ui->defaultMemory->setText(QString::number(qreal(m_defaultMemory)/div));
    ui->defaultMap->setText(QString::number(qreal(m_defaultMap)/div));
    ui->defaultDisk->setText(m_defaultDisk==(u_int64_t)-1?QString("unlimited"):QString::number(qreal(m_defaultDisk)/div));
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
    ui->valueDisk->setText(currentDisk==(u_int64_t)-1?QString("unlimited"):QString::number(qreal(currentDisk)/div));
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

    QString filename = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
            + "/config.json";
    QFile file(filename);
    if ( !file.open(QIODevice::ReadOnly)) {
        if (create) {
            dflWarning("Configuration file doesn't exist, creating one");
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
    if ( area <= 0 )
        ui->valueArea->setText("unlimited");
    else
        ui->valueArea->setText(QString::number(mul*area));
    if ( memory <= 0 )
        ui->valueMemory->setText("unlimited");
    else
        ui->valueMemory->setText(QString::number(mul*memory));
    if ( map <= 0 )
        ui->valueMap->setText("unlimited");
    else
        ui->valueMap->setText(QString::number(mul*map));
    if ( disk <= 0 )
        ui->valueDisk->setText("unlimited");
    else
        ui->valueDisk->setText(QString::number(mul*disk));
    if ( threads <= 0 )
        ui->valueThreads->setText("unlimited");
    else
        ui->valueThreads->setText(QString::number(threads));

    setMagickResources();

    m_currentTarget = TransformTarget(pixels["transformTarget"].toInt());
    ui->comboTransformTarget->setCurrentIndex(m_currentTarget);
    m_incompatibleAction = IncompatibleAction(pixels["incompatibleAction"].toInt());
    ui->comboIncompatibleScale->setCurrentIndex(m_incompatibleAction);
    m_labSelectionSize = pixels["labSelectionSize"].toInt();
    ui->spinLabSelectionSize->setValue(m_labSelectionSize);

    ui->valueTmpDir->setText(path["tmp"].toString());
    ui->valueBaseDir->setText(path["base"].toString());

    MagickCore::ExceptionInfo *exception = MagickCore::AcquireExceptionInfo();
    MagickCore::SetImageRegistry(MagickCore::StringRegistryType, "temporary-path", (const void*)ui->valueTmpDir->text().toLocal8Bit(), exception);
    dflDebug("magick tmp = %s", (const char*)MagickCore::GetImageRegistry(MagickCore::StringRegistryType, "temporary-path", exception));
    ui->comboLogLevel->setCurrentIndex(logging["level"].toInt());
    ui->comboRaiseOn->setCurrentIndex(logging["raise"].toInt());
    ui->comboTrapOn->setCurrentIndex(logging["trap"].toInt());

    Console::setLevel(Console::Level(logging["level"].toInt()));
    Console::setRaiseLevel(Console::Level(logging["raise"].toInt()));
    Console::setTrapLevel(Console::Level(logging["trap"].toInt()));
    return true;
}

void Preferences::save()
{
    QJsonObject resources;
    QJsonObject pixels;
    QJsonObject path;
    QJsonObject logging;

    QString areaStr = ui->valueArea->text();
    QString memoryStr = ui->valueMemory->text();
    QString mapStr = ui->valueMap->text();
    QString diskStr = ui->valueDisk->text();
    QString threadsStr = ui->valueThreads->text();

    const u_int64_t mul = 1<<30;

    resources["area"] =  qint64((areaStr.isEmpty()||areaStr=="unlimited")?-1:areaStr.toDouble()*mul);
    resources["memory"] = qint64((memoryStr.isEmpty()||memoryStr=="unlimited")?-1:memoryStr.toDouble()*mul);
    resources["map"] = qint64((mapStr.isEmpty()||mapStr=="unlimited")?-1:mapStr.toDouble()*mul);
    resources["disk"] = qint64((diskStr.isEmpty()||diskStr=="unlimited")?-1:diskStr.toDouble()*mul);
    resources["threads"] = qint64((threadsStr.isEmpty()||threadsStr=="unlimited")?-1:threadsStr.toLongLong());

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
    QJsonDocument doc;
    doc.setObject(obj);

    QString filename = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
            + "/config.json";
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if ( !dir.exists() ) {
        if ( !dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)) ) {
            dflCritical("Unable to create configuration directory");
            return;
        }
    }
    QFile saveFile(filename);
    if ( !saveFile.open(QIODevice::WriteOnly) ) {
        dflCritical("Could not save configuration file.");
        return;
    }
    saveFile.write(doc.toJson());
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

int Preferences::getLabSelectionSize() const
{
    return m_labSelectionSize;
}
