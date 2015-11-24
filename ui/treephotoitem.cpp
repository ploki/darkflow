#include "treephotoitem.h"
#include "photo.h"


TreePhotoItem::TreePhotoItem(const Photo &photo,
                             PhotoType type,
                             QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_photo(photo),
    m_type(type)
{
    setText(0, photo.getTag("Name"));
    setToolTip(0, photo.getIdentity());
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

bool TreePhotoItem::isInput() const
{
    return m_type == Input;
}
