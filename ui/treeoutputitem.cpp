#include "treeoutputitem.h"
#include "operator.h"
#include "operatoroutput.h"

TreeOutputItem::TreeOutputItem(OperatorOutput *output,
                               Role role,
                               QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_output(output)
{
    if ( Source == role )
        setText(0, m_output->m_operator->getName() + ": " + m_output->name());
    else
        setText(0, m_output->name());
    setFont(0, QFont("Sans", 12));
    setBackground(0, QBrush(Qt::yellow));
}

TreeOutputItem::~TreeOutputItem()
{
}

OperatorOutput *TreeOutputItem::output() const
{
    return m_output;
}
