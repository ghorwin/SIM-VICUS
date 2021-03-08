#include "SVDBModelDelegate.h"

#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyleOptionViewItemV4>

#include "SVStyle.h"
#include "SVConstants.h"

SVDBModelDelegate::SVDBModelDelegate(QObject * parent, int builtInRole) :
	QItemDelegate(parent),
	m_builtInRole(builtInRole)
{
}

SVDBModelDelegate::~SVDBModelDelegate() {
}

void SVDBModelDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	const QStyleOptionViewItem * opt = &option;
	// if color column, just fill the background with the color and be gone
	if (index.data(Role_Color).toBool()) {
		// draw background
		QBrush b(index.data(Qt::BackgroundRole).value<QColor>());
		painter->fillRect(option.rect, b);
		return;
	}
	// find out if our index is of a built-in element
	bool builtin = index.data(m_builtInRole).toBool();
	bool enabled = opt->widget->isEnabled();
	if (builtin && enabled) {
		// draw background
		QBrush b;
		if (opt->features & QStyleOptionViewItem::Alternate)
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
