#include "operatorparameter.h"

OperatorParameter::OperatorParameter(
        const QString& name,
        const QString& caption,
        QObject *parent) :
    QObject(parent),
    m_name(name),
    m_caption(caption)
{

}
QString OperatorParameter::caption() const
{
    return m_caption;
}
QString OperatorParameter::name() const
{
    return m_name;
}



