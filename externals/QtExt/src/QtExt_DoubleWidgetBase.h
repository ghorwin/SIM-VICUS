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

#ifndef QtExt_DoubleWidgetBaseH
#define QtExt_DoubleWidgetBaseH

#include <QWidget>
#include <QHBoxLayout>

namespace QtExt {


/*! \brief Widget base class to display two widgets and toggle the visibility of both.

	DoubleWidgetBase includes two widgets and sets the default layout properties.
	One of both widget can be displayed. DoubleWidgetBase includes a toggle function
	to set the visibility to the other widget. You have to give the specific widget
	in the constructor of this class.
*/
class DoubleWidgetBase : public QWidget {
	Q_OBJECT
public:
	/*! Constructor.
		\param widget1 First widget.
		\param widget2 Second widget.
		\param parent Parent widget.
		The class sets the ownership of both widget to the parent of this class.
		The widget should not be deleted externally.
	*/
	DoubleWidgetBase(QWidget* widget1, QWidget* widget2, QWidget *parent);

	/*! Default destructor. */
	~DoubleWidgetBase();

	/*! Returns true if the first widget is visible and false if the second widget is visible. */
	bool isFirstVisible() { return m_widget1->isVisibleTo(this); }

public slots:
	/*! Toggels the visibility of the two widgets and emits widgetsChanged(). */
	void toggleWidgets();

	/*! Sets the visibility of the first widget.
		The visibility of the second one will be set to the contrary.
		The signal widgetsChanged() is only emitted when the visible of the
		widgets has changed.
		\param visible Visibility of the first widget.
	*/
	void setFirstVisible(bool visible);

signals:
	/*! Fires when the widgets has changed its visibility. */
	void widgetsChanged();

protected:
	/*! Widget to display plot data. */
	QWidget		*m_widget1;

	/*! Widget to display data. */
	QWidget		*m_widget2;
};

} // namespace QtExt

#endif // QtExt_DoubleWidgetBaseH
