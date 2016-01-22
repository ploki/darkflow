#ifndef TREEOUTPUTITEM_H
#define TREEOUTPUTITEM_H

#include <QTreeWidgetItem>
#include <QCoreApplication>
class OperatorOutput;

class TreeOutputItem : public QTreeWidgetItem
{
    Q_DECLARE_TR_FUNCTIONS(TreeOutputItem)
public:
    typedef enum {
        Source,
        EnabledSink,
        DisabledSink
    } Role;
    enum { Type = QTreeWidgetItem::UserType + 1 };
    explicit TreeOutputItem(OperatorOutput *output,
                            int idx,
                            Role role,
                            QTreeWidgetItem *parent = 0);

    ~TreeOutputItem();
    OperatorOutput *output() const;

    int idx() const;
    Role role() const;
    void setRole(const Role &role);
    void setCaption();

private:
    OperatorOutput *m_output;
    int m_idx;
    Role m_role;
    QString m_caption;
};

#endif // TREEOUTPUTITEM_H
