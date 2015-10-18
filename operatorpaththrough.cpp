#include "operatorpaththrough.h"

OperatorPathThrough::OperatorPathThrough(Process *parent) :
    Operator(parent)
{
}

OperatorPathThrough::~OperatorPathThrough()
{
   // qWarning((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}


OperatorPathThrough *OperatorPathThrough::newInstance()
{
    return new OperatorPathThrough(m_process);
}

QString OperatorPathThrough::getClassIdentifier()
{
   return "Pass Through";
}


Image *OperatorPathThrough::process(const Image *image)
{
    return const_cast<Image*>(image);
}
