/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDBModelDelegate.h"

#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyleOptionViewItemV4>

#include "SVStyle.h"
#include "SVConstants.h"

#include <VICUS_ZoneTemplate.h>

SVDBModelDelegate::SVDBModelDelegate(QObject * parent, int builtInRole) :
	QItemDelegate(parent),
	m_builtInRole(builtInRole)
{
}

SVDBModelDelegate::SVDBModelDelegate(QObject * parent, int builtInRole, int localRole) :
	QItemDelegate(parent),
	m_builtInRole(builtInRole),
	m_localRole(localRole)
{
}

SVDBModelDelegate::~SVDBModelDelegate() {
}


void drawSubTemplateBar( QPainter * painter, QStyleOptionViewItem & option, int subTemplateType) {
	QBrush b(QColor(VICUS::KeywordList::Color("ZoneTemplate::SubTemplateType", subTemplateType)));

	const int BARWIDTH = 10;
	QRect r(option.rect);
	r.setWidth(BARWIDTH);

	option.rect.setLeft(option.rect.left()+BARWIDTH);
	painter->fillRect(r, b);
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
	bool local = index.data(m_localRole).toBool();
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

		// if we are in SubTemplateType-column and have a valid subtemplate, we draw a colored bar based on
		// subtemplate type
		QVariant subTemplateType = index.data(Role_SubTemplateType);
		if (subTemplateType.isValid() && index.column() == 1) // note: column index is currently hard-coded
			drawSubTemplateBar(painter, modifiedOption, subTemplateType.toInt());
		QItemDelegate::paint(painter, modifiedOption, index);
	}

	// TODO Andreas : check this ... (more or less copy & paste)

	else if (!local) {
		QBrush b;
		if (opt->features & QStyleOptionViewItem::Alternate)
			b = QBrush("#cddafd");
		else
			b = QBrush("#dfe7fd");
		painter->fillRect(option.rect, b);
		// adjust text color for subsequent call to QItemDelegate::paint()
		QPalette pal = opt->palette;
		pal.setColor(QPalette::Text, SVStyle::instance().m_alternativeBackgroundText);
		QStyleOptionViewItem modifiedOption(option);
		modifiedOption.palette = pal;

		QItemDelegate::paint(painter, modifiedOption, index);
	}
	else {
		QVariant subTemplateType = index.data(Role_SubTemplateType);
		QStyleOptionViewItem modifiedOption(option);
		if (subTemplateType.isValid() && index.column() == 1)
			drawSubTemplateBar(painter, modifiedOption, subTemplateType.toInt());
		QItemDelegate::paint(painter, modifiedOption, index);
	}
}
