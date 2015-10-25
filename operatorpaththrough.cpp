#include "operatorpaththrough.h"
#include "operatorinput.h"

OperatorPathThrough::OperatorPathThrough(Process *parent) :
    Operator(parent)
{
    m_inputs.push_back(new OperatorInput("Images set 1","Images set # one",OperatorInput::Set));
    m_inputs.push_back(new OperatorInput("Images set 2","Images set # two",OperatorInput::Set));
    m_inputs.push_back(new OperatorInput("Images set 3","Images set # three",OperatorInput::Set));
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
