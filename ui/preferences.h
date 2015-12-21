#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
class Preferences;
}
class QAbstractButton;
class QSemaphore;
class QMutex;
class OperatorWorker;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    typedef enum {
        Linear,
        sRGB,
        IUT_BT_709,
        SquareRoot,
        TargetNone
    } TransformTarget;

    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

    QString baseDir();

    bool acquireWorker(OperatorWorker *worker);
    void releaseWorker();

    TransformTarget getCurrentTarget() const;

private:
    void getDefaultMagickResources();
    void getMagickResources();
    void setMagickResources();
    bool load(bool create=true);
    void save();

public slots:
    void clicked(QAbstractButton * button);
    void accept();
    void reject();
    void tmpDirClicked();
    void baseDirClicked();

private:
    Ui::Preferences *ui;
    size_t m_defaultArea;
    size_t m_defaultMemory;
    size_t m_defaultMap;
    size_t m_defaultDisk;
    size_t m_defaultThreads;
    QSemaphore *m_sem;
    QMutex *m_mutex;
    size_t m_currentMaxWorkers;
    size_t m_scheduledMaxWorkers;
    size_t m_OpenMPThreads;
    TransformTarget m_currentTarget;

};

extern Preferences *preferences;

#endif // PREFERENCES_H
