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

#ifndef QtExt_DateTimeInputDialogH
#define QtExt_DateTimeInputDialogH

#include <QDialog>

namespace Ui {
	class DateTimeInputDialog;
}

namespace QtExt {

/*! A dialog to request date or date/time inputs from user.

	TODO : configure to request only date, only time, a mix, add date/time limits, ....

*/
class DateTimeInputDialog : public QDialog {
	Q_OBJECT

public:
	explicit DateTimeInputDialog(QWidget *parent = nullptr);
	~DateTimeInputDialog();

	/*! Creates the dialog
		\code
		QDate dt = requestDate("Enter start date");
		QDate dt2 = requestDate("Enter start date", "MM/dd");
		\endcode

		\return Returns a valid date/time object if entered, otherwise invalid object when dialog was cancelled.
	*/
	static QDate requestDate(const QString & title, const QString & label, const QString & dateFormat = "dd.MM.", QDate * initialValue = nullptr);

private:
	::Ui::DateTimeInputDialog *m_ui;
};


} // namespace QtExt

#endif // QtExt_DateTimeInputDialogH
