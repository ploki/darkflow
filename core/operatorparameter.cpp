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
    connect(this, SIGNAL(setOutOfDate()), m_operator, SLOT(setOutOfDate()), Qt::QueuedConnection);
}
QString OperatorParameter::caption() const
{
    return m_caption;
}
QString OperatorParameter::name() const
{
    return m_name;
}



