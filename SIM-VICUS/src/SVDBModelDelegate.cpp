#include "SVDBModelDelegate.h"

#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyleOptionViewItemV4>

#include "SVStyle.h"

SVDBModelDelegate::SVDBModelDelegate(QObject * parent, int builtInRole) :
	QItemDelegate(parent),
	m_builtInRole(builtInRole)
{
}

SVDBModelDelegate::~SVDBModelDelegate() {
}

void SVDBModelDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	const QStyleOptionViewItem * opt = &option;
	// find out if our index is of a built-in element
	bool builtin = index.data(m_builtInRole).toBool();
	if (builtin) {
		// draw background
		QBrush b;
		if (opt->features & QStyleOptionViewItemV2::Alternate)
			b = QBrush(SVStyle::instance().m_alternativeBackgroundDark);
		else
			b = QBrush(SVStyle::instance().m_alternativeBackgroundBright);
		painter->fillRect(option.rect, b);
		// adjust text color for subsequent call to QItemDelegate::paint()
		QPalette pal = opt->palette;
		pal.setColor(QPalette::Text, SVStyle::instance().m_alternativeBackgroundText);
		QStyleOptionViewItem modifiedOption(option);
		modifiedOption.palette = pal;
		QItemDelegate::paint(painter, modifiedOption, index);
	}
	else
		QItemDelegate::paint(painter, option, index);
}
