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

#include "QtExt_Directories.h"

#include <QCoreApplication>
#if QT_VERSION < 0x050000
	#include <QDesktopServices>
#else
	#include <QStandardPaths>
#endif
#include <QDir>
#include <QSettings>

#include <IBK_configuration.h>
#include <IBK_messages.h>

namespace QtExt {

// You need to set values to these variables early on in your main.cpp
QString Directories::appname;		// for example "Therakles"
QString Directories::devdir;		// for example "TheraklesApp"
QString Directories::packagename;	// for example "therakles" or "therakles-pro"

/*! Utility function for conversion of a QString to a trimmed std::string in utf8 encoding. */
inline std::string QString2trimmedUtf8(const QString & str) {
	return str.trimmed().toStdString();
}


QString Directories::resourcesRootDir() {
	QString installPath = qApp->applicationDirPath();

#if defined(IBK_DEPLOYMENT) || defined(IBK_BUILDING_DEBIAN_PACKAGE)
	// deployment mode

#if defined(Q_OS_WIN)
	// in Deployment mode, resources are below install directory
	return installPath + "/resources";
#elif defined(Q_OS_MAC)
	// in deployment mode, we have them in <appname>.app/Contents/Resources
	// where install path is <appname>.app/MacOS
	return installPath + "/../Resources";
#elif defined(Q_OS_UNIX)

#ifdef IBK_BUILDING_DEBIAN_PACKAGE

	// we install to /usr/bin/<appname>
	// and the package data is in
	//               /usr/share/<packagename>
	return installPath + "/../share/" + packagename;

#else // IBK_BUILDING_DEBIAN_PACKAGE

	return installPath + "/../resources";

#endif // IBK_BUILDING_DEBIAN_PACKAGE

#endif // defined(Q_OS_UNIX)


#else // IBK_DEPLOYMENT

	// development (IDE) mode

	// resources are expected in devdir/resources directory
#if defined(Q_OS_MAC)
	// in development mode, we have the resources outside the bundle
	return QFileInfo(installPath + "/../../../../" + devdir + "/resources").absoluteFilePath();
#else
	return QFileInfo(installPath + "/../../" + devdir + "/resources").absoluteFilePath();
#endif

#endif // IBK_DEPLOYMENT
}


QString Directories::databasesDir() {
#ifdef IBK_DEPLOYMENT

	// deployment mode, databases are in the same directory as resources
	return resourcesRootDir();

#else // IBK_DEPLOYMENT

	// during development, databases are expected in the data directory
	QString installPath = qApp->applicationDirPath();

#if defined(Q_OS_WIN)
	// in development mode, we have the resources in the data directory
	// executables are build in bin/debug or bin/release
	return installPath + "/../../data";
#elif defined(Q_OS_MAC)
	// in development mode, we have the resources outside the bundle
	return installPath + "/../../../../../data";
#elif defined(Q_OS_UNIX)
	return installPath + "/../../data";
#else
#error Implement this for your platform/compiler
#endif

#endif // IBK_DEPLOYMENT
}



QString Directories::translationsFilePath(const QString & langID) {
#ifdef IBK_BUILDING_DEBIAN_PACKAGE
	QString installPath = qApp->applicationDirPath();
	return installPath + QString("/../share/locale/%1/LC_MESSAGES/"+appname+".qm").arg(langID);
#else // IBK_BUILDING_DEBIAN_PACKAGE
	return resourcesRootDir() + QString("/translations/"+appname+"_%1.qm").arg(langID);
#endif // IBK_BUILDING_DEBIAN_PACKAGE
}


QString Directories::qtTranslationsFilePath(const QString & langID) {
#if defined(Q_OS_LINUX)
	return QString("/usr/share/qt5/translations/qt_%1.qm").arg(langID);
#else
	// in all other cases the qt_xx.qm files are located in the resources path
	return resourcesRootDir() + QString("/translations/qt_%1.qm").arg(langID);
#endif
}


QString Directories::userDataDir() {
	// we have different user data directories, based on OS
#if defined(Q_OS_WIN)
	// on Windows, we store user data unter %HOME%/AppData/Roaming
	QString fname = QDir::toNativeSeparators(QDir::home().absolutePath() + "/AppData/Roaming/" + appname);
#else
	// on Unix/Mac OS we store user data under home directory
	QString fname = QDir::toNativeSeparators(QDir::home().absolutePath() + "/.local/share/" + appname);
#endif // Q_OS_WIN
	QFileInfo fpath(fname);
	return fpath.absoluteFilePath();
}


QString Directories::userExampleFileDir() {
	return QDir::homePath() + "/" + appname + "/examples";
}


QString Directories::tmpDir() {
#if defined(Q_OS_WIN)
	// on Windows, we store user data unter %TMP%
#if QT_VERSION < 0x050000
	return QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#else // QT_VERSION < 0x050000
	return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#endif // QT_VERSION < 0x050000
#else
	// on Unix/Mac OS we use global tmp dir
	return "/tmp";
#endif // Q_OS_WIN
}


QString Directories::globalLogFile() {
	return userDataDir() + "/"+appname+".log";
}


QString Directories::generateTempFileName() {

	const char * const FUNC_ID = "[Directories::generateTempFileName]";

	QString tmpDirectory = tmpDir();

	unsigned int i = 0;
	for (;;) {
		++i;
		// generate file path
		QString fpath = tmpDirectory + QString("/ibk_tmp%1").arg(i);
		// check if file exists
		if (QFileInfo(fpath).exists())
			continue; // next index
		// check if file can be written into
		if (!checkForWriteAccess(fpath)) {
			IBK::IBK_Message( IBK::FormatString("Cannot write to temporary file '%1'").arg( QString2trimmedUtf8(fpath) ),
							  IBK::MSG_WARNING, FUNC_ID);
			break; // no write access to tmp
		}
		return fpath; // file name found
	}

	// cannot write in global tmp, use tmp subdir in user data dir
	tmpDirectory = userDataDir() + "/tmp";
	if (!QDir(tmpDirectory).exists()) {
		if (!QDir().mkdir(tmpDirectory)) {
			IBK::IBK_Message( IBK::FormatString("Cannot create user-tmp directory '%1'").arg( QString2trimmedUtf8(tmpDirectory) ),
							  IBK::MSG_ERROR, FUNC_ID);
			return QString();
		}
	}

	for (;;) {
		++i;
		// generate file path
		QString fpath = tmpDirectory + QString("/ibk_tmp%1").arg(i);
		// check if file exists
		if (QFileInfo(fpath).exists())
			continue; // next index
		// check if file can be written into
		if (!checkForWriteAccess(fpath)) {
			IBK::IBK_Message( IBK::FormatString("Cannot write to temporary file '%1'").arg( QString2trimmedUtf8(fpath) ),
							  IBK::MSG_ERROR, FUNC_ID);
			return QString();
		}
		return fpath; // file name found
	}
}


bool Directories::checkForWriteAccess(const QString & newFileName) {
	if (QFile(newFileName).exists())
		return false;
	// check if file can be written into
	QFile f(newFileName);
	if (f.open(QIODevice::WriteOnly)) {
		f.remove();
		return true;
	}
	return false;
}


QString Directories::updateFilePath() {
	// In user-data-dir 'updates' check for
#ifdef _WIN32
	// on Windows, we store user data unter %HOME%/AppData/Local
	QString fname = QDir::toNativeSeparators(QDir::home().absolutePath() + "/AppData/Local/" + appname + "/updates");
	return fname;
#elif defined(Q_OS_MAC)
	// "version" will be replaced by new version number
	return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#else
	// "version" will be replaced by new version number
	return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
}


// helper function to remove a complete directory hierarchie
bool Directories::removeDirRecursively(const QString & directory) {
	QDir dir(directory);

	if (!dir.exists(directory))
		return false;

	// entryInfoList() filter options taken from John's Blog, http://http://john.nachtimwald.com, June 8th, 2010
	QFileInfoList fInfoList = dir.entryInfoList(
		QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs |
		QDir::Files, QDir::DirsFirst);
	foreach(QFileInfo fInfo, fInfoList) {
		if (fInfo.isDir()) {
			if (!removeDirRecursively(fInfo.absoluteFilePath()))
				return false;
		}
		else {
			if (!QFile::remove(fInfo.absoluteFilePath()))
				return false;
		}
	}
	if (!dir.rmdir(directory))
		return false;

	return true;
}

} // namespace QtExt
