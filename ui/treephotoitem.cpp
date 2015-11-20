#include "treephotoitem.h"
#include "photo.h"


TreePhotoItem::TreePhotoItem(const Photo &photo, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_photo(photo)
{
    setText(0, photo.getTag("Name"));
    if ( !photo.isComplete() ) {
        qWarning("photo is not complete");
    }
    if ( !m_photo.isComplete() ) {
        qWarning("m_photo is not complete");
    }
}

const Photo &TreePhotoItem::photo() const
{
    return m_photo;
}

Photo &TreePhotoItem::photo()
{
    return m_photo;
}
