#include "treephotoitem.h"
#include "photo.h"


TreePhotoItem::TreePhotoItem(const Photo &photo, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_photo(new Photo(photo))
{
    setText(0, photo.getTag("Name"));
}

TreePhotoItem::~TreePhotoItem()
{
    delete m_photo;
}

Photo *TreePhotoItem::photo() const
{
    return m_photo;
}
