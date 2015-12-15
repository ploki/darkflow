#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QAbstractButton>
#include <QFileDialog>

#include "preferences.h"
#include "ui_preferences.h"
#include "console.h"

#include <Magick++.h>

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


Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences),
  m_defaultArea(0),
  m_defaultMemory(0),
  m_defaultMap(0),
  m_defaultDisk(0),
  m_defaultThreads(0)
{
    ui->setupUi(this);

    ui->comboResourcesUnit->setCurrentIndex(1);
    getDefaultMagickResources();
    getMagickResources();

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

    load();
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::getDefaultMagickResources()
{
    int unit = ui->comboResourcesUnit->currentIndex();
    size_t div;
    if ( unit == 0 )
        div = 1<<20;
    else
        div = 1<<30;
    m_defaultArea    = Magick::ResourceLimits::area();
    m_defaultMemory  = Magick::ResourceLimits::memory();
    m_defaultMap     = Magick::ResourceLimits::map();
    m_defaultDisk    = Magick::ResourceLimits::disk();
    m_defaultThreads = Magick::ResourceLimits::thread();
    ui->defaultArea->setText(QString::number(qreal(m_defaultArea)/div));
    ui->defaultMemory->setText(QString::number(qreal(m_defaultMemory)/div));
    ui->defaultMap->setText(QString::number(qreal(m_defaultMap)/div));
    ui->defaultDisk->setText(m_defaultDisk==(size_t)-1?QString("unlimited"):QString::number(qreal(m_defaultDisk)/div));
    ui->defaultThreads->setText(QString::number(m_defaultThreads));
}

void Preferences::getMagickResources()
{
    int unit = ui->comboResourcesUnit->currentIndex();
    size_t div;
    if ( unit == 0 )
        div = 1<<20;
    else
        div = 1<<30;
    size_t currentArea    = Magick::ResourceLimits::area();
    size_t currentMemory  = Magick::ResourceLimits::memory();
    size_t currentMap     = Magick::ResourceLimits::map();
    size_t currentDisk    = Magick::ResourceLimits::disk();
    size_t currentThreads = Magick::ResourceLimits::thread();
    ui->valueArea->setText(QString::number(qreal(currentArea)/div));
    ui->valueMemory->setText(QString::number(qreal(currentMemory)/div));
    ui->valueMap->setText(QString::number(qreal(currentMap)/div));
    ui->valueDisk->setText(currentDisk==(size_t)-1?QString("unlimited"):QString::number(qreal(currentDisk)/div));
    ui->valueThreads->setText(QString::number(currentThreads));
}

void Preferences::setMagickResources()
{
    int unit = ui->comboResourcesUnit->currentIndex();
    size_t div;
    if ( unit == 0 )
        div = 1<<20;
    else
        div = 1<<30;
    size_t currentArea    = ui->valueArea->text().toDouble()*div;
    size_t currentMemory  = ui->valueMemory->text().toDouble()*div;
    size_t currentMap     = ui->valueMap->text().toDouble()*div;
    size_t currentDisk    = ui->valueDisk->text().toDouble()*div;
    size_t currentThreads = ui->valueThreads->text().toDouble();
    Magick::ResourceLimits::area(currentArea);
    Magick::ResourceLimits::memory(currentMemory);
    Magick::ResourceLimits::map(currentMap);
    Magick::ResourceLimits::disk(currentDisk);
    Magick::ResourceLimits::thread(currentThreads);
    getMagickResources();
}

void Preferences::load()
{
    int unit = ui->comboResourcesUnit->currentIndex();
    qreal mul;
    if ( unit == 0 )
        mul = 1./(1<<20);
    else
        mul = 1./(1<<30);

    QString filename = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
            + "/config.json";
    QFile file(filename);
    if ( !file.open(QIODevice::ReadOnly)) {
        dflWarning("Configuration file doesn't exist, creating one");
        save();
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject obj = doc.object();
    QJsonObject resources = obj["resources"].toObject();
    QJsonObject path = obj["path"].toObject();
    QJsonObject logging = obj["logging"].toObject();

    ssize_t area = resources["area"].toDouble();
    ssize_t memory = resources["memory"].toDouble();
    ssize_t map = resources["map"].toDouble();
    ssize_t disk = resources["disk"].toDouble();
    ssize_t threads = resources["threads"].toDouble();
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
}

void Preferences::save()
{
    QJsonObject resources;
    QJsonObject path;
    QJsonObject logging;

    QString areaStr = ui->valueArea->text();
    QString memoryStr = ui->valueMemory->text();
    QString mapStr = ui->valueMap->text();
    QString diskStr = ui->valueDisk->text();
    QString threadsStr = ui->valueThreads->text();

    int unit = ui->comboResourcesUnit->currentIndex();
    size_t mul;
    if ( unit == 0 )
        mul = 1<<20;
    else
        mul = 1<<30;

    resources["area"] =  qint64((areaStr.isEmpty()||areaStr=="unlimited")?-1:areaStr.toDouble()*mul);
    resources["memory"] = qint64((memoryStr.isEmpty()||memoryStr=="unlimited")?-1:memoryStr.toDouble()*mul);
    resources["map"] = qint64((mapStr.isEmpty()||mapStr=="unlimited")?-1:mapStr.toDouble()*mul);
    resources["disk"] = qint64((diskStr.isEmpty()||diskStr=="unlimited")?-1:diskStr.toDouble()*mul);
    resources["threads"] = qint64((threadsStr.isEmpty()||threadsStr=="unlimited")?-1:threadsStr.toLongLong());

    path["tmp"] = ui->valueTmpDir->text();
    path["base"] = ui->valueBaseDir->text();

    logging["level"] = ui->comboLogLevel->currentIndex();
    logging["raise"] = ui->comboRaiseOn->currentIndex();
    logging["trap"] = ui->comboTrapOn->currentIndex();

    QJsonObject obj;
    obj["resources"] = resources;
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
