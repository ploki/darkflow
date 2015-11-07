#include "operator.h"
#include "operatorinput.h"

OperatorInput::OperatorInput(const QString &name,
                             const QString &description,
                             OperatorInput::OperatorInputCompatibility compatibility,
                             Operator *parent) :
    QObject(parent),
    m_operator(parent),
    m_name(name),
    m_description(description),
    m_compatibility(compatibility),
    m_sources()
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
QSet<OperatorOutput *> OperatorInput::sources() const
{
    return m_sources;
}

void OperatorInput::addSource(OperatorOutput *output)
{
    m_sources.insert(output);
}

void OperatorInput::removeSource(OperatorOutput *output)
{
    m_sources.remove(output);
}

