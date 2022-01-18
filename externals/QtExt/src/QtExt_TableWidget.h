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

#ifndef QtExt_TableWidgetH
#define QtExt_TableWidgetH

#include <QWidget>

/*! \brief Namespace for Qt Extension lib.*/
/*! The namespace QtExt includes all classes and functions for Qt extensions.
*/
namespace QtExt {

	class Table;

/*! \brief Class TableWidget allows to create and draw a table with HTML-formated text.*/
class TableWidget : public QWidget
{
Q_OBJECT
public:
	/*! Default constructor.*/
	explicit TableWidget(QtExt::Table* table, QWidget *parent = 0);
	Table* table();

protected:
	virtual void paintEvent ( QPaintEvent * event );	///< Paint event. Calls drawTable.
	virtual void resizeEvent ( QResizeEvent * event );	///< Rezize event. Does nothing.

signals:

public slots:
	void repaintTable();

private:
	Table* m_table;

};


/*! @file QtExt_TableWidget.h
  @brief Contains the declaration of the class TableWidget.
*/

} // namespace QtExt
#endif // QtExt_TableWidgetH
