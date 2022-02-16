/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#include "QtExt_MaterialDBItemDelegate.h"

#include <QPainter>

#include "QtExt_MaterialBase.h"
#include "QtExt_Style.h"

namespace QtExt {


MaterialDBItemDelegate::MaterialDBItemDelegate(QObject *parent) :
	QItemDelegate(parent),
	m_alternatingColors(true),
	m_defaultColors(true),
	m_colorUserMaterials(Qt::white),
	m_colorAlternativeBackgroundBright(Style::AlternativeBackgroundBright),
	m_colorAlternativeBackgroundDark(Style::AlternativeBackgroundDark)
{
}

void MaterialDBItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	// first copy options in order make changes possible
	QStyleOptionViewItem opt = option;

	// draw pixmap for capability colors
	MaterialBase::parameter_t para = static_cast<MaterialBase::parameter_t>(index.data(ParaRole).toInt());
	if(para == MaterialBase::MC_PARAMETRIZATION) {
		QVariant data = index.data(Qt::DecorationRole);
		QPixmap pixmap = data.value<QPixmap>();
		if (!pixmap.isNull()) {
			int x, y, width, height;
			opt.rect.getRect(&x, &y, &width, &height);
			width = std::min(width, pixmap.width());
			painter->drawPixmap(x,y, width, height, pixmap);
		}
	}
	// set alternating colors
	else if(m_alternatingColors) {
		// find out if our index is of a built-in element
		bool builtin = index.data(BuiltInRole).toBool();
		if (builtin) {
			// draw background
			QBrush b;
			if (opt.features & QStyleOptionViewItemV2::Alternate)
				b = QBrush(m_colorAlternativeBackgroundDark);
			else
				b = QBrush(m_colorAlternativeBackgroundBright);
			painter->fillRect(option.rect, b);
			// adjust text color for subsequent call to QItemDelegate::paint()
			QPalette pal = opt.palette;
			pal.setColor(QPalette::Text, Style::AlternativeBackgroundText);
			opt.palette = pal;
		}
		else {
			// white color for user materials
			painter->fillRect(opt.rect, QBrush(m_colorUserMaterials));
		}
	}
	QItemDelegate::paint(painter, opt, index);
}

void MaterialDBItemDelegate::setDefaultColors() {
	m_defaultColors = true;
	m_colorAlternativeBackgroundBright = Style::AlternativeBackgroundBright;
	m_colorAlternativeBackgroundDark = Style::AlternativeBackgroundDark;
}

void MaterialDBItemDelegate::setUserDefinedColors(const QColor& bright, const QColor& dark) {
	m_defaultColors = false;
	m_colorAlternativeBackgroundBright = bright;
	m_colorAlternativeBackgroundDark = dark;
}

void MaterialDBItemDelegate::setAlternatingColors(bool alternate) {
	m_alternatingColors = alternate;
}

void MaterialDBItemDelegate::setUserColor(const QColor& col) {
	m_colorUserMaterials = col;
}


} // end namespace

