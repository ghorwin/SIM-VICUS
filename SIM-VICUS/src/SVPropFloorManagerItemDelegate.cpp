#include "SVPropFloorManagerItemDelegate.h"

#include <QPainter>
#include <QEvent>

#include <QtExt_Locale.h>

SVPropFloorManagerItemDelegate::SVPropFloorManagerItemDelegate(QWidget * parent) :
	QItemDelegate(parent)
{
}


QWidget * SVPropFloorManagerItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	if (index.parent() == QModelIndex() && index.column() != 0)
		return nullptr;
	return QItemDelegate::createEditor(parent, option, index);
}

