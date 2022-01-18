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

#include "QtExt_DatabaseReference.h"

#include <QStringList>

namespace QtExt {

DatabaseReference::DatabaseReference() :
	m_location(LocalDirectory)
{
}

DatabaseReference::DatabaseReference(DatabaseLocation loc, const QString & typeStr,
									 const QString & desc, const QUrl & url) :
	m_location(loc),
	m_typeString(typeStr),
	m_description(desc),
	m_url(url)
{
}


QString DatabaseReference::toString() const {
	// encode database reference in format:
	// [LOCAL_DIRECTORY|Material|Default Material Database] file://path/to/database/directory
	QString encoded = "[";
	switch (m_location) {
		case LocalDirectory		: encoded += "LOCAL_DIRECTORY|"; break;
		case Webservice			: encoded += "WEBSERVICE|"; break;
		default					: encoded += "UNKNOWN|"; break;
	}
	encoded += m_typeString + "|" + m_description + "] " + m_url.toString();
	return encoded;
}


DatabaseReference DatabaseReference::fromString(const QString & str) {
	DatabaseReference dbRef;
	// split string into parts
	int pos2 = str.indexOf("]");
	int pos1 = str.indexOf("[");
	if (pos2 < pos1 || pos1 != 0 || pos2 == -1) {
		return dbRef; // invalid reference
	}
	QString url = str.right(str.size() - pos2 - 1);
	dbRef.m_url = QUrl(url.trimmed());
	QString remainder = str.mid(1, pos2-1);
	QList<QString> tokens = remainder.split("|");
	if (tokens.size() != 3) {
		return dbRef;
	}
	if (tokens[0] == "LOCAL_DIRECTORY")
		dbRef.m_location = LocalDirectory;
	else if (tokens[0] == "WEBSERVICE")
		dbRef.m_location = Webservice;
	else return dbRef;
	dbRef.m_typeString = tokens[1];
	dbRef.m_description = tokens[2];
	return dbRef;
}


} // namespace QtExt
