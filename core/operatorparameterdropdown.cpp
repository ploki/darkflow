#include <QMenu>
#include "operatorparameterdropdown.h"

OperatorParameterDropDown::OperatorParameterDropDown(
        const QString& name,
        const QString& caption,
        Operator *op,
        const char *slot) :
    OperatorParameter(name, caption, op),
    m_menu(new QMenu),
    m_currentValue()
{
    connect(this, SIGNAL(itemSelected(int)), op,slot);
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
}

OperatorParameterDropDown::~OperatorParameterDropDown()
{
    delete m_menu;
}

void OperatorParameterDropDown::addOption(const QString &option,
                                          int value,
                                          bool selected)
{
    m_options[option] = value;
    m_menu->addAction(QIcon(), option);
    if (selected)
        m_currentValue = option;
}

void OperatorParameterDropDown::dropDown(const QPoint& pos)
{
    m_menu->exec(pos);
}

void OperatorParameterDropDown::actionTriggered(QAction *action)
{
    m_currentValue=action->text();
    emit itemSelected(m_options[m_currentValue]);
    emit valueChanged(m_currentValue);
}

QString OperatorParameterDropDown::currentValue() const
{
    return m_currentValue;
}

QJsonObject OperatorParameterDropDown::save()
{
    QJsonObject obj;
    obj["type"] = "dropdown";
    obj["name"] = m_name;
    obj["currentValue"] = m_currentValue;
    return obj;
}

void OperatorParameterDropDown::load(const QJsonObject &obj)
{
    if ( obj["type"].toString() != "dropdown" ) {
        qWarning("invalid parameter type");
        return;
    }
    if ( obj["name"].toString() != m_name ) {
        qWarning("invalid parameter name");
        return;
    }
    bool actionFound = false;
    QString currentValue = obj["currentValue"].toString();
    foreach(QAction *action, m_menu->actions()) {
        if ( action->text() == currentValue ) {
            actionFound = true;
            action->trigger();
            break;
        }
    }
    if (!actionFound)
        qWarning("unknown value for dropdown");
}


