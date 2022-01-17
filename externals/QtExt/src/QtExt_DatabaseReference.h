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

#ifndef QtExt_DatabaseReferenceH
#define QtExt_DatabaseReferenceH

#include <QString>
#include <QUrl>

namespace QtExt {

/*! This class references a database (directory). */
class DatabaseReference {
public:

	/*! The database location. */
	enum DatabaseLocation {
		/*! Standard type of data source, materials are located in a directory. */
		LocalDirectory,
		/*! Special data source type, is only populated temporary when material files
			are stored relative to the project file. */
		DirectoryRelativeToProjectFile,
		/*! Data is provided through web-service-query. */
		Webservice
	};

	/*! Default constructor, creates an empty reference to a LocalDirectory db. */
	DatabaseReference();

	/*! Initializing constructor.
		\param loc Location of the database, either as a standard materials are located in a directory, or as a special data source type, is only populated temporary when material files are stored relative to the project file.
		\param typeStr A string describing the type of the database, for example "Material", or "Construction".
		\param desc A description shown to the user.
		\param url URL pointing to the database/service.
	*/
	DatabaseReference(DatabaseLocation loc, const QString & typeStr,
		const QString & desc, const QUrl & url);

	/*! Returns a string encoding the database reference. */
	QString toString() const;

	/*! Creates a database reference object from an encoded string. */
	static DatabaseReference fromString(const QString & str);



	/*! Location of the database. */
	DatabaseLocation			m_location;
	/*! A string describing the type of the database, for example "Material", or "Construction". */
	QString						m_typeString;
	/*! A description shown to the user. */
	QString						m_description;
	/*! URL pointing to the database/service. */
	QUrl						m_url;

};


} // namespace QtExt


#endif // QtExt_DatabaseReferenceH
