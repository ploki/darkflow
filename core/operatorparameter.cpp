#include "operatorparameter.h"

OperatorParameter::OperatorParameter(
        const QString& name,
        const QString& caption,
        Operator *op) :
    QObject(op),
    m_operator(op),
    m_name(name),
    m_caption(caption)
{
    connect(this, SIGNAL(setUpToDate(bool)), m_operator, SLOT(setUpToDate(bool)));
}
QString OperatorParameter::caption() const
{
    return m_caption;
}
QString OperatorParameter::name() const
{
    return m_name;
}



