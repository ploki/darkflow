#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
class Preferences;
}
class QAbstractButton;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

private:
    void getDefaultMagickResources();
    void getMagickResources();
    void setMagickResources();
    void load();
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

};

#endif // PREFERENCES_H
