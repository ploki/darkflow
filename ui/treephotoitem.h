#ifndef TREEPHOTOITEM_H
#define TREEPHOTOITEM_H

#include <QTreeWidgetItem>
#include "photo.h"

class TreePhotoItem : public QTreeWidgetItem
{
public:
    enum { Type = QTreeWidgetItem::UserType + 2 };
    typedef enum {
        InputEnabled,
        InputDisabled,
        InputReference,
        Output,
    } PhotoType;
    explicit TreePhotoItem(const Photo& photo,
                           PhotoType type,
                           QTreeWidgetItem *parent = 0);

    const Photo &photo() const;
    Photo &photo();
    bool isInput() const;
    void setType(PhotoType type);

private:
    Photo m_photo;
    PhotoType m_type;
};

#endif // TREEPHOTOITEM_H
