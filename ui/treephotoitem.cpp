#include "treephotoitem.h"
#include "photo.h"
#include "console.h"

TreePhotoItem::TreePhotoItem(const Photo &photo,
                             PhotoType type,
                             QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_photo(photo),
    m_type(type)
{
    setText(0, photo.getTag(TAG_NAME));
    setToolTip(0, photo.getIdentity());
    if ( !photo.isComplete() ) {
        dflCritical(TreePhotoItem::tr("TreePhotoItem: photo is not complete"));
    }
    if ( !m_photo.isComplete() ) {
        dflCritical(TreePhotoItem::tr("TreePhotoItem: m_photo is not complete"));
    }
    setType(type);
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
    return m_type != Output;
}

void TreePhotoItem::setType(TreePhotoItem::PhotoType type)
{
    if ( type == InputError ) {
        setForeground(0, Qt::red);
    }
    else if ( type == InputDisabled ) {
        setForeground(0, Qt::gray);
    }
    else if ( type == InputReference ) {
        setForeground(0, Qt::green);
    }
    else {
        setForeground(0, Qt::black);
    }

}
