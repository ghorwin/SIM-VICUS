/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
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

