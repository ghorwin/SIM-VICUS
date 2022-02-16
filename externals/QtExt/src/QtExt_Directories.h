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



	/*! Returns the platform-specific root directory of all read-only resource files. */
	static QString resourcesRootDir();

	/*! Returns the platform-specific directory of all read-only translation files. */
	static QString databasesDir();

	/*! Returns the platform-specific directory of all read-only translation files. */
	static QString translationsDir();

	/*! Returns the platform-specific root directory of all user database files. */
	static QString userDataDir();

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
