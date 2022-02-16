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

#include "QtExt_DateTimeInputDialog.h"
#include "ui_QtExt_DateTimeInputDialog.h"

namespace QtExt {

DateTimeInputDialog::DateTimeInputDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::DateTimeInputDialog)
{
	m_ui->setupUi(this);
}


DateTimeInputDialog::~DateTimeInputDialog() {
	delete m_ui;
}


QDate DateTimeInputDialog::requestDate(const QString & title, const QString & label, const QString & dateFormat, QDate * initialValue) {
	// create and configure dialog

	DateTimeInputDialog dlg(nullptr); // top level

	if (title.isEmpty())
		dlg.setWindowTitle(qApp->applicationName());
	else
		dlg.setWindowTitle(title);

	dlg.m_ui->label->setText(label);
	dlg.m_ui->dateEdit->setDisplayFormat(dateFormat);
	if (initialValue != nullptr && initialValue->isValid())
		dlg.m_ui->dateEdit->setDate(*initialValue);

	int res = dlg.exec();

	if (res != QDialog::Accepted)
		return QDate(); // return invalid object
	else
		return dlg.m_ui->dateEdit->date();
}

} // namespace QtExt
