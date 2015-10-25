#include <QMenu>
#include "operatorparameterdropdown.h"

OperatorParameterDropDown::OperatorParameterDropDown(const QString& caption,
                                                     const QString& currentValue,
                                                     QObject *parent) :
    OperatorParameter(parent),
    m_menu(new QMenu),
    m_caption(caption),
    m_currentValue(currentValue)
{
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
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

void OperatorParameterDropDown::actionTriggered(QAction *action)
{
    m_currentValue=action->text();
    emit valueChanged(m_currentValue);
}

QString OperatorParameterDropDown::caption() const
{
    return m_caption;
}

QString OperatorParameterDropDown::currentValue() const
{
    return m_currentValue;
}


