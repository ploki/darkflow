#include "tablewidgetitem.h"
#include "tabletagsrow.h"

TableWidgetItem::TableWidgetItem(const QString &label, FieldType fieldType, TableTagsRow *row) :
    QTableWidgetItem(label),
    m_row(row),
    m_fieldType(fieldType)
{}

void TableWidgetItem::setData(int role, const QVariant &value) {
    QTableWidgetItem::setData(role,value);
    if ( role == Qt::EditRole)
        m_row->changed(m_fieldType);
}

TableTagsRow *TableWidgetItem::tableRow()
{
    return m_row;
}
