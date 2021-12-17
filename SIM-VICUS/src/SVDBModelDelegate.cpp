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

SVDBModelDelegate::SVDBModelDelegate(QObject * parent, int builtInRole, int localRole, int referencedRole) :
	QItemDelegate(parent),
	m_builtInRole(builtInRole),
	m_localRole(localRole),
	m_referencedRole(referencedRole)
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
	// we have three different background color modes: built-in, user-db and local
	// if the widget is disabled, we use the default background (darkish gray)
	// if the widget is enabled, we have special colors for builtin and for local.
	bool builtin = index.data(m_builtInRole).toBool();
	bool enabled = opt->widget->isEnabled();
	bool local = index.data(m_localRole).toBool();
	bool referenced = index.data(m_referencedRole).toBool();

	// if we are in SubTemplateType-column and have a valid subtemplate, we draw a colored bar based on
	// subtemplate type - this is independent of the DB type
	QStyleOptionViewItem modifiedOption(option);
	QVariant subTemplateType = index.data(Role_SubTemplateType);
	if (subTemplateType.isValid() && index.column() == 1)
		drawSubTemplateBar(painter, modifiedOption, subTemplateType.toInt()); // we modify the rect property here

	// modify font style
	QPalette pal = opt->palette;
	QFont font = opt->font;
	if (referenced) {
		font.setBold(true);
		modifiedOption.font = font;
	}
	modifiedOption.palette = pal;
	modifiedOption.font = font;

	// local and builtin are exclusive - we can only have either one
	if (builtin && enabled) {
		// draw background
		QBrush b;
		if (opt->features & QStyleOptionViewItem::Alternate)
			b = QBrush(SVStyle::instance().m_alternativeBackgroundDark);
		else
			b = QBrush(SVStyle::instance().m_alternativeBackgroundBright);
		painter->fillRect(modifiedOption.rect, b);
	}
	else if (!local && enabled) {
		QBrush b;
		if (opt->features & QStyleOptionViewItem::Alternate)
			b = QBrush(SVStyle::instance().m_userDBBackgroundDark);
		else
			b = QBrush(SVStyle::instance().m_userDBBackgroundBright);
		painter->fillRect(modifiedOption.rect, b);
	}
	// either disable or user-DB
	QItemDelegate::paint(painter, modifiedOption, index);
}
