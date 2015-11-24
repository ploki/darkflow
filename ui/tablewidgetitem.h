#ifndef TABLEWIDGETITEM_H
#define TABLEWIDGETITEM_H

#include <QTableWidgetItem>

class TableTagsRow;

class TableWidgetItem : public QTableWidgetItem {
public:
    typedef enum {
        Key,
        Value,
    } FieldType;
    TableWidgetItem(const QString& label, FieldType fieldType, TableTagsRow *row);

    void setData(int role, const QVariant &value);
    TableTagsRow *tableRow();

private:
    TableTagsRow *m_row;
    FieldType m_fieldType;
};


#endif // TABLEWIDGETITEM_H
