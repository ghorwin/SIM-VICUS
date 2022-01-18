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

#include "QtExt_ComboBoxCountries.h"

#include <QFile>
#include <QTextStream>

namespace QtExt {

ComboBoxCountries::ComboBoxCountries(QWidget *parent) :
	QComboBox (parent)
{

	 QFile file(":/text/countries.txt");
	 file.open(QIODevice::ReadOnly);
	 QTextStream txtStrm(&file);
	 QStringList list;
	 QString str;
	 do {
		str = txtStrm.readLine();
		if(!str.isEmpty())
			list << str;
	 } while(str != QString());

	 addItem("", tr("None"));
	 foreach (const QString& str, list) {
		 QStringList tl = str.split('|');
		 Q_ASSERT(tl.size() == 2);
		 addItem(tl[1], tl[0]);
	 }
	 setEditable(false);
}

QString ComboBoxCountries::countryName() const {
	return currentText();
}

QString ComboBoxCountries::countryShort() const {
	return currentData().toString();
}

void ComboBoxCountries::setCountry(const QString& country, bool fullName) {
	if(fullName) {
		int index = findText(country);
		if(index == -1) {
			addItem(country);
			setCurrentIndex(count() - 1);
		}
		else {
			setCurrentIndex(index);
		}
	}
	else {
		int index = findData(country.toUpper());
		if(index >= 0)
			setCurrentIndex(index);
		else
			setCountry("Germany");
	}
}

} // namespace QtExt
