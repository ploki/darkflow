#include <QMenu>
#include "operatorparameterdropdown.h"

OperatorParameterDropDown::OperatorParameterDropDown(QObject *parent) :
    OperatorParameter(parent),
    m_menu(new QMenu)
{
}

OperatorParameterDropDown::~OperatorParameterDropDown()
{
    delete m_menu;
}

void OperatorParameterDropDown::addOption(const QString &option,
                                          QObject *obj,
                                          const char *slot)
{
    m_menu->addAction(QIcon(), option, obj, slot);
}

void OperatorParameterDropDown::dropDown(const QPoint& pos)
{
    m_menu->exec(pos);
}

