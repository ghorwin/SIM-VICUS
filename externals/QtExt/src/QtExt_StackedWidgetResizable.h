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

#ifndef QtExt_StackedWidgetResizableH
#define QtExt_StackedWidgetResizableH

#include <QStackedWidget>

namespace QtExt {

/*! \brief Derivation of QStacked widget with changed sizing behaviour.
	Ther base class creates its size from the biggest internal widget. This class uses only the currently visible widget for size.
*/
class StackedWidgetResizable : public QStackedWidget
{
	Q_OBJECT
public:
	/*! Standard constructor. Connects the currentChanged signal with the internal slot.*/
	StackedWidgetResizable(QWidget* parent = nullptr);

	/*! Redefinition of addWidget method in order to change the size policies of the widget to ignored.
		This removes the widget fron sizeHint calculation.
	*/
	void addWidget(QWidget* pWidget);

	/*! Returns sizeHint of current widget.*/
	QSize sizeHint() const override;

	/*! Returns minimumSizeHint of current widget.*/
	QSize minimumSizeHint() const override;

private slots:
	/*! Connected with currentChanged signal in order to set size policy of current widget to preferred.
		It calles adjustSize for recalculation of complete size.
	*/
	void onCurrentChanged(int index);
};

} // end namespace

#endif // QtExt_StackedWidgetResizableH
