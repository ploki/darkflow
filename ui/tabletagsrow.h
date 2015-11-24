#ifndef TABLETAGSROW_H
#define TABLETAGSROW_H

#include <QObject>
#include "tablewidgetitem.h"

class QTableWidget;
class TableWidgetItem;
class Operator;

class TableTagsRow : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        FromPhoto,
        FromOperator,
    } Source;
    explicit TableTagsRow(const QString& photoIdentity,
                          const QString& key,
                          const QString& value,
                          Source source,
                          QTableWidget *tableWidget,
                          Operator *op);
    ~TableTagsRow();

    void reset();
    bool remove();
    void setValue(const QString& value, bool initial = false);
    QString getKey() const;

private:
    QString m_photoIdentity;
    TableWidgetItem *m_key;
    QString m_previousKey;
    TableWidgetItem *m_value;
    QString m_originalValue;
    QTableWidget *m_table;
    Source m_source;
    bool m_dirty;
    Operator *m_operator;
    bool m_initDone;

private slots:
    friend class TableWidgetItem;
    void setColors();
    void changed(TableWidgetItem::FieldType fieldType);
};

#endif // TABLETAGSROW_H
