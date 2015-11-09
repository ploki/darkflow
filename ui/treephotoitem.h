#ifndef TREEPHOTOITEM_H
#define TREEPHOTOITEM_H

#include <QTreeWidgetItem>

class Photo;

class TreePhotoItem : public QTreeWidgetItem
{
public:
    enum { Type = QTreeWidgetItem::UserType + 2 };
    explicit TreePhotoItem(const Photo& photo, QTreeWidgetItem *parent = 0);
    ~TreePhotoItem();

    Photo *photo() const;

private:
    Photo *m_photo;
};

#endif // TREEPHOTOITEM_H
