#include "tabletagsrow.h"
#include <QTableWidget>
#include "tablewidgetitem.h"
#include "operator.h"

TableTagsRow::TableTagsRow(const QString& photoIdentity,
                           const QString &key,
                           const QString &value,
                           TableTagsRow::Source source,
                           QTableWidget *tableWidget,
                           Operator *op) :
    QObject(NULL),
    m_photoIdentity(photoIdentity),
    m_key(new TableWidgetItem(key, TableWidgetItem::Key, this)),
    m_previousKey(key),
    m_value(new TableWidgetItem(value, TableWidgetItem::Value, this)),
    m_originalValue(value),
    m_table(tableWidget),
    m_source(source),
    m_dirty(false),
    m_operator(op),
    m_initDone(false)
{
    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow(rowCount);
    tableWidget->setItem(rowCount, 0, m_key);
    tableWidget->setItem(rowCount, 1, m_value);
    if ( m_source == FromPhoto )
        m_key->setFlags(Qt::NoItemFlags);
    m_initDone=true;
    setColors();
}

TableTagsRow::~TableTagsRow()
{
    m_table->removeRow(m_key->row());
}

void TableTagsRow::reset()
{
    if ( m_source == FromPhoto ) {
        m_value->setText(m_originalValue);
        m_dirty = false;
        m_operator->resetTagOverride(m_photoIdentity,
                                     m_key->text());
        setColors();
    }
}

bool TableTagsRow::remove()
{
    bool ret = false;
    switch(m_source) {
    case FromPhoto:
        setValue("");
        ret = true;
        break;
    case FromOperator:
        ret = false;
        m_operator->resetTagOverride(m_photoIdentity, m_key->text());
        break;
    }
    return ret;
}

void TableTagsRow::setValue(const QString& value, bool initial)
{
    m_value->setText(value);
    if ( m_source == FromPhoto ) {
        m_dirty = true;
    }
    setColors();
    if ( !initial )
        changed(TableWidgetItem::Value);
}

QString TableTagsRow::getKey() const
{
    return m_key->text();
}

void TableTagsRow::setColors()
{
    QColor color;
    switch(m_source) {
    case FromPhoto:
        if ( m_dirty )
            color = Qt::darkRed;
        else
            color = Qt::black;
        break;
    case FromOperator:
        color = Qt::darkGreen;
        break;
    }
    m_key->setTextColor(color);
    m_value->setTextColor(color);

}

void TableTagsRow::changed(TableWidgetItem::FieldType fieldType)
{
    if (!m_initDone) return;
    if ( fieldType == TableWidgetItem::Key &&
         m_source == FromOperator ) {
        m_operator->resetTagOverride(m_photoIdentity, m_previousKey);
        m_previousKey = m_key->text();
    }
    m_operator->setTagOverride(m_photoIdentity,
                               m_key->text(),
                               m_value->text());
    if (m_source == FromPhoto)
        m_dirty = true;
    setColors();
}
