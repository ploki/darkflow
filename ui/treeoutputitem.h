#ifndef TREEOUTPUTITEM_H
#define TREEOUTPUTITEM_H

#include <QTreeWidgetItem>

class OperatorOutput;

class TreeOutputItem : public QTreeWidgetItem
{
public:
    typedef enum {
        Source,
        Sink
    } Role;
    enum { Type = QTreeWidgetItem::UserType + 1 };
    explicit TreeOutputItem(OperatorOutput *output,
                            Role role,
                            QTreeWidgetItem *parent = 0);

    ~TreeOutputItem();
    OperatorOutput *output() const;

private:
    OperatorOutput *m_output;
};

#endif // TREEOUTPUTITEM_H
