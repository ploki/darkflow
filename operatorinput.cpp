#include "operatorinput.h"

OperatorInput::OperatorInput(const QString &name,
                             const QString &description,
                             OperatorInput::OperatorInputCompatibility
                             compatibility) :
    m_name(name),
    m_description(description),
    m_compatibility(compatibility)
{}

QString OperatorInput::name() const
{
    return m_name;
}

QString OperatorInput::description() const
{
    return m_description;
}

OperatorInput::OperatorInputCompatibility
OperatorInput::compatibility() const
{
    return m_compatibility;
}
