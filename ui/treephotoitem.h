#ifndef TREEPHOTOITEM_H
#define TREEPHOTOITEM_H

#include <QTreeWidgetItem>
#include "photo.h"

class TreePhotoItem : public QTreeWidgetItem
{
public:
    enum { Type = QTreeWidgetItem::UserType + 2 };
    explicit TreePhotoItem(const Photo& photo, QTreeWidgetItem *parent = 0);

    const Photo &photo() const;
    Photo &photo();

private:
    Photo m_photo;
};

#endif // TREEPHOTOITEM_H
