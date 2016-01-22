#include "treeoutputitem.h"
#include "operator.h"
#include "operatoroutput.h"
#include <QObject>

TreeOutputItem::TreeOutputItem(OperatorOutput *output,
                               int idx,
                               Role role,
                               QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_output(output),
    m_idx(idx),
    m_role(role),
    m_caption()
{
    if ( Source == role )
        m_caption = m_output->m_operator->getName() + ": " + m_output->name();
    else
        m_caption = m_output->name();
    setCaption();
}

void TreeOutputItem::setCaption()
{
    setFont(0, QFont("Sans", 12));
    if ( Source == m_role ) {
        setBackground(0, QBrush(Qt::yellow));
        setText(0, m_caption);
    }
    else if ( EnabledSink == m_role ) {
        setBackground(0, QBrush(Qt::yellow));
        setText(0, tr("☑ %0").arg(m_caption));
    }
    else {
        setBackground(0, QBrush(Qt::gray));
        setText(0, tr("☐ %0").arg(m_caption));
    }
}

void TreeOutputItem::setRole(const Role &role)
{
    m_role = role;
}

TreeOutputItem::~TreeOutputItem()
{
}

OperatorOutput *TreeOutputItem::output() const
{
    return m_output;
}

int TreeOutputItem::idx() const
{
    return m_idx;
}

TreeOutputItem::Role TreeOutputItem::role() const
{
    return m_role;
}

