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

#include "QtExt_DoubleWidgetBase.h"

#include <IBK_messages.h>

#include <QHBoxLayout>

#include <QTableView>
#include <QListView>

namespace QtExt {

DoubleWidgetBase::DoubleWidgetBase(QWidget* widget1, QWidget* widget2, QWidget *parent) :
	QWidget(parent),
	m_widget1(widget1),
	m_widget2(widget2)
{

	m_widget1->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
	m_widget1->setParent(parent);
	m_widget2->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
	m_widget2->setParent(parent);

	QHBoxLayout *hBox = new QHBoxLayout;
	hBox->addWidget(m_widget1);
	hBox->addWidget(m_widget2);
	hBox->setMargin(0);
	hBox->setSpacing(0);
	setLayout(hBox);

	// first widget is always visible at first
	m_widget1->setVisible(true);
	m_widget2->setVisible(false);
}

DoubleWidgetBase::~DoubleWidgetBase()
{
}

void DoubleWidgetBase::toggleWidgets() {
	setFirstVisible( !m_widget1->isVisibleTo(this) );
}

void DoubleWidgetBase::setFirstVisible(bool visible) {
	const char * const FUNC_ID = "[DoubleWidgetBase::setFirstVisible]";
	// check if first widget is already visible
	if (m_widget1->isVisibleTo(this) == visible)
		return; // already visible, no need to change anything
	IBK::IBK_Message(IBK::FormatString("Setting first widget %1.\n").arg(visible ? "visible" : "invisible"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
	m_widget1->setVisible(visible);
	m_widget2->setVisible(!visible);
	emit widgetsChanged();
	IBK::IBK_Message(IBK::FormatString("Toggling visibility done.\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
	return;
}

} //widgetsChanged
