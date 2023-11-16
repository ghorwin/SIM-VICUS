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

#ifndef QtExt_DirectoriesH
#define QtExt_DirectoriesH

#include <QString>

namespace QtExt {

/*! Provides default locations for resource and user data files based on IBK_DEPLOYMENT
	flag setting and OS.

	Use this as base class for application-specific directory management.
*/
class Directories {
public:
	/*! This directory name is appended to all platform-specific locations.
		For example 'Delphin6' will generate a database save location: /home/<user>/.local/share/Delphin6
		or C:\Users\<user>\AppData\Roaming\Delphin6 on Windows.
	*/
	static QString appname;
	/*! The top-level directory within the source code directory tree, where resources and translations can be found.
		For example 'TheraklesApp' will be used to generate the translation dir: `resourcesRootDir() + "/../TheraklesApp/resources/translations"`
	*/
	static QString devdir;
	/*! Name of the Linux/Unix package, determines install location of resources.
		For example 'delphin6' will generate a resource: /usr/share/delphin6
	*/
	static QString packagename;


	/*! Returns the platform-specific root directory of all read-only resource files. */
	static QString resourcesRootDir();

	/*! Returns the platform-specific directory of all read-only translation files. */
	static QString databasesDir();

	/*! Returns the path to the application's translation file path. */
	static QString translationsFilePath(const QString & langID);

	/*! Returns the path to the Qt translation file path. */
	static QString qtTranslationsFilePath(const QString & langID);

	/*! Returns the platform-specific root directory of all user database files. */
	static QString userDataDir();

	/*! Returns the platform-specific user directory, where examples can be stored. */
	static QString userExampleFileDir();

	/*! Returns the platform-specific temporary directory. */
	static QString tmpDir();

	/*! Returns path to global log file. */
	static QString globalLogFile();

	/*! Generates a new temporary file path (file not yet existing) and tests writing into this file.
		If writing into file is not possible, an empty string is returned.
	*/
	static QString generateTempFileName();

	/*! This function opens the file in "write" mode and removes the file afterwards again to test if writing is possible.
		\return True, if file can be written into. False if the file already exists, or if file cannot be opened for writing.

		\note Function expects parent directory to exist.
	*/
	static bool checkForWriteAccess(const QString & newFileName);

	/*! Local file path to update-installer (without filename). */
	static QString updateFilePath();

	/*! Qt4 function for removing directory structures recursively, in Qt5 use QDir::removeRecursively(). */
	static bool removeDirRecursively(const QString & directory);
};

}

#endif // QtExt_DirectoriesH
