#ifndef TREEPHOTOITEM_H
#define TREEPHOTOITEM_H

#include <QCoreApplication>
#include <QTreeWidgetItem>
#include "photo.h"

class TreePhotoItem : public QTreeWidgetItem
{
    Q_DECLARE_TR_FUNCTIONS(TreePhotoItem)
public:
    enum { Type = QTreeWidgetItem::UserType + 2 };
    typedef enum {
        InputEnabled,
        InputDisabled,
        InputReference,
        InputError,
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
