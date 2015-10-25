#ifndef OPERATORPARAMETERDROPDOWN_H
#define OPERATORPARAMETERDROPDOWN_H

#include <QString>
#include <QPoint>

#include "operatorparameter.h"

class QObject;
class QMenu;

class OperatorParameterDropDown : public OperatorParameter
{
public:
    OperatorParameterDropDown(QObject *parent = 0);
    ~OperatorParameterDropDown();

    void addOption(const QString& option, QObject *obj, const char *slot);
    void dropDown(const QPoint& pos);

private:
    QMenu *m_menu;
};

#endif // OPERATORPARAMETERDROPDOWN_H
