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
