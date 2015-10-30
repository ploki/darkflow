#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "image.h"

OperatorOutput::OperatorOutput(const QString &name,
                               const QString &description,
                               Operator *parent) :
    QObject(parent),
    m_operator(parent),
    m_name(name),
    m_description(description),
    m_sinks(),
    m_result()
{

}

OperatorOutput::~OperatorOutput()
{
    foreach(Image *im, m_result)
        delete im;
}

QString OperatorOutput::name() const
{
    return m_name;
}
QString OperatorOutput::description() const
{
    return m_description;
}

QSet<OperatorInput *> OperatorOutput::sinks() const
{
    return m_sinks;
}

void OperatorOutput::addSink(OperatorInput *input)
{
    m_sinks.insert(input);
}

void OperatorOutput::removeSink(OperatorInput *input)
{
    m_sinks.remove(input);
}

QVector<Image *> OperatorOutput::getResult() const
{
    return m_result;
}


