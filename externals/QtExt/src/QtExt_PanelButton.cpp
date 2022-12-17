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

#include "QtExt_PanelButton.h"

#include <QDebug>
#include <QApplication>

#include "QtExt_Constants.h"

namespace QtExt {


PanelButton::PanelButton( QWidget * parent, Direction direction ):
	QToolButton( parent )
{

	QIcon iconBtn;

	switch ( direction ) {
		case PanelButton::DD_NORTH :
		{
			setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_top/03_arrows_flat_top_enabled_24x24.png")), QIcon::Normal, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_top/04_arrows_flat_top_hover_24x24.png")), QIcon::Active, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_bottom/05_arrows_flat_bottom_enabled_checked_24x24.png")), QIcon::Normal, QIcon::Off);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_bottom/06_arrows_flat_bottom_hover_checked_24x24.png")), QIcon::Active, QIcon::Off);
			setIcon(iconBtn);
			m_direction = PanelButton::DD_NORTH;
			setMaximumHeight(PANEL_BUTTON_SIZE);
		} break;
		case PanelButton::DD_SOUTH :
			setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_bottom/03_arrows_flat_bottom_enabled_24x24.png")), QIcon::Normal, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_bottom/04_arrows_flat_bottom_hover_24x24.png")), QIcon::Active, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_top/05_arrows_flat_top_enabled_checked_24x24.png")), QIcon::Normal, QIcon::Off);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_top/06_arrows_flat_top_hover_checked_24x24.png")), QIcon::Active, QIcon::Off);
			setIcon(iconBtn);
			m_direction = PanelButton::DD_SOUTH;
			setMaximumHeight(PANEL_BUTTON_SIZE);
			break;
		case PanelButton::DD_EAST :
			setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_right/03_arrows_flat_right_enabled_24x24.png")), QIcon::Normal, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_right/04_arrows_flat_right_hover_24x24.png")), QIcon::Active, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_left/05_arrows_flat_left_enabled_checked_24x24.png")), QIcon::Normal, QIcon::Off);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_left/06_arrows_flat_left_hover_checked_24x24.png")), QIcon::Active, QIcon::Off);
			setIcon(iconBtn);
			m_direction = PanelButton::DD_EAST;
			setMaximumWidth(PANEL_BUTTON_SIZE);
			break;
		case PanelButton::DD_WEST :
			setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_left/03_arrows_flat_left_enabled_24x24.png")), QIcon::Normal, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_left/04_arrows_flat_left_hover_24x24.png")), QIcon::Active, QIcon::On);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_right/05_arrows_flat_right_enabled_checked_24x24.png")), QIcon::Normal, QIcon::Off);
			iconBtn.addPixmap(QPixmap(QString::fromUtf8(":/gfx/panels/arrow_flat_right/06_arrows_flat_right_hover_checked_24x24.png")), QIcon::Active, QIcon::Off);
			setIcon(iconBtn);
			m_direction = PanelButton::DD_WEST;
			setMaximumWidth(PANEL_BUTTON_SIZE);
			break;

	}

	setCheckable(true);
	setAutoRaise(true);
	retranslateUi();
}


void PanelButton::retranslateUi() {

#if QT_VERSION >= 0x050000
// Qt5 code
	switch(m_direction){
		case PanelButton::DD_WEST:
			setShortcut(QApplication::translate("PanelButton", "Alt+3", 0));
		break;
		case PanelButton::DD_EAST:
			setShortcut(QApplication::translate("PanelButton", "Alt+4", 0));
			break;
		case PanelButton::DD_SOUTH:
			setShortcut(QApplication::translate("PanelButton", "Alt+2", 0));
			break;
		case PanelButton::DD_NORTH:
			setShortcut(QApplication::translate("PanelButton", "Alt+1", 0));
			break;
	}
#else
// Qt4 code
	switch(m_direction){
		case PanelButton::DD_WEST:
			setShortcut(QApplication::translate("PanelButton", "Alt+3", 0, QApplication::UnicodeUTF8));
		break;
		case PanelButton::DD_EAST:
			setShortcut(QApplication::translate("PanelButton", "Alt+4", 0, QApplication::UnicodeUTF8));
			break;
		case PanelButton::DD_SOUTH:
			setShortcut(QApplication::translate("PanelButton", "Alt+2", 0, QApplication::UnicodeUTF8));
			break;
		case PanelButton::DD_NORTH:
			setShortcut(QApplication::translate("PanelButton", "Alt+1", 0, QApplication::UnicodeUTF8));
			break;
	}
#endif


}

} // namespace QtExt
