#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <QDialog>

namespace Ui {
class ProjectProperties;
}

class QString;
class Process;

class ProjectProperties : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectProperties(QWidget *parent = 0);

    void modify(Process *process);
    void modifyAndSave(Process *process);
    ~ProjectProperties();


public slots:
    void selectProjectFile();
    void selectBaseDirectory();
    void accept();

private:
    Ui::ProjectProperties *ui;
    Process *m_process;
    bool m_andSave;
    void modifyInternal(Process *process, bool andSave);
};

#endif // PROJECTPROPERTIES_H
