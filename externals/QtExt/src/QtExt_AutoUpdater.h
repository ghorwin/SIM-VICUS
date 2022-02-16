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

#ifndef QtExt_AutoUpdaterH
#define QtExt_AutoUpdaterH

#include <QObject>
#include <QMap>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;
class QUrl;

namespace QtExt {

/*! A class that implements all functionality needed for auto-update features.

	Specifically, it implements downloading the update-info file from a given server, extract and compare version
	number and provide the changelog text. Then, it implements downloading the update file and storing it on
	the local machine.
	Finally, it provides the functionality for checking for existing update file, executing it in a separate process
	and marking the update as installed for cleanup during next call.

	Example for use-case 1, start update installer when available:
	\code
	QtExt::AutoUpdater updater;
	if (updater.installUpdateWhenAvailable("/path/to/update_installer.exe"))
		return EXIT_SUCCESS; // close application so that installer can overwrite installed files
	\endcode

	Example for use-case 2, check and display available update info. In this example the auto-updater handles all
	cases:
	* no connection possible (gives out message "Cannot retrieve update information.").
	* no newer version available (gives out message "Your software is up-to-date.").
	* a newer version available (shows the "Update available" dialog. Starts the download of the changelog file and
	  once completed, shows the changelog in the update available dialog. When user clicks on "download", the download
	  of the appropriate installer file is started. Once download is complete, user is prompted to install
	 (starts update with the help of use-case 1).
	\code
	QtExt::AutoUpdater updater;
	updater.checkForUpdateInfo("http://www.bauklimatik-dresden.de/downloads/delphin6/Delphin6.updates");
	\endcode
	For the example above to work, the update-info file is expected to have a certain format:
	\code
	linux:Delphin_6.0.6_linux_2017-01-06.7z
	win:Delphin_6.0.5_win_int.exe
	win64:Delphin_6.0.5_win64_int.exe
	mac:Delphin_6.0.6_osx10.7.4_macosx_int.dmg
	---HeaderEnd---
	...changlog text follows...
	\endcode
	The file paths in above update info file are expexted to be relative to the path were D6.updates resides in
	or full-fledged URLs. For the installer files, the prefix (before the first underscore) is expected to be followed
	by a full version number to be compared against. The keys in the key-value list are platform indicators.

	Example for use-case 3: try to retrieve update information return most recent version number.
	\code
	QtExt::AutoUpdater updater;
	connect(&updater, SIGNAL(updateInfoAvailable()), this, SLOT(onUpdateInfoAvailable()));
	updater.checkForUpdateInfo("http://www.bauklimatik-dresden.de/downloads/delphin6/Delphin6.updates");
	// within the slot 'onUpdateInfoAvailable()'

	switch (updater.m_updateInfoResult) {
		case AutoUpdater::UpdateInfoSuccess :
			std::cout << "New version = " << updater.m_newVersionNumber.toStdString() << std::endl;
			break;
		case AutoUpdater::ConnectionFailure : ;
		case AutoUpdater::NoNewVersion :
			std::cout << "No new version available" << std::endl;
			break; // can't update
	}
	\endcode
*/
class AutoUpdater : public QObject {
	Q_OBJECT
public:
	explicit AutoUpdater(QObject *parent = nullptr);

#if defined(Q_OS_WIN)
	/*! Call this function from main.cpp to check if a downloaded update installer is available on the specified
		location.
		In fact, any executable in the given path is treated like an update installer, moved to a temporary directory
		and executed in a new process.
		To ensure that really only the correct executable file is run, you may pass an md5hash of the file's content.
		Then, the function will open all *.exe files in the
		\return Returns true, if a process with the update file was started (in this case the calling program should
			terminate right away).
	*/
	static bool installUpdateWhenAvailable(const QString & localPath, const std::string & md5hash = "");
#endif // defined(Q_OS_WIN)


	/*! Attempts to download the update-info file from a given URL and returns true if
		the download was successful.

		The member variables then contain the info needed to integrate the update information
		into the application.
		\code
		// look for updates file
		autoUpdater->checkForUpdateInfo(QUrl("http://www.bauklimatik-dresden.de/downloads/delphin6/Delphin6.updates"), "6.0.7");
		\endcode

		\param url The download url for the update-info file.
		\param LONG_VERSION Three digit version number, used to check if there is a new version available.
		\param interactive If true, a dialog is shown after the update info has been retrieved (or an error message dialog if
			it failed), that shows new version and change log (downloaded separately). If false, function emits the signal
			updateInfoRetrieved(). The receiver of the signal can then check and compare the version number and either
			start the update dialog manually, or show any other means of update notification.
		\param downloadFilePath Path, where downloaded update install shall be stored in. Filename will be the same as the downloaded file.
		\param newestRejectedVersion Only applicable when "interactive=false", i.e. when

		\warning On Windows it is possible, that the current user of DELPHIN does not have install privileges. When installing
			the update, the user account is switched and the user-specific application download path changes. As such,
			the downloaded update_xxx.exe file is not found and not deleted by the admin user. As a consequence, next time
			the user runs DELPHIN he/she is requested to update again. To avoid this, the file is first renamed before run.
	*/
	void checkForUpdateInfo(const QString & url, const char * const LONG_VERSION, bool interactive, QString downloadFilePath,
							QString newestRejectedVersion);

	/*! Shows a message box saying that there is no newer version available and shows the changelog file that
		was downloaded from the webpage.
	*/
	void showChangelogInfo(QWidget * w);

	/*! Shows a dialog with update info, using the content of the member variables m_newVersion and m_changeLogText for display
		in the dialog.
		Updates m_newestRejectedVersion when update was skipped.
	*/
	static void showUpdateDialog(QWidget * parent, QString currentVersion, QString newVersion,
								 QString changeLogText, QString downloadUrl, QString downloadFilePath,
								 std::string & downloadedFileMD5, bool & rejected);

	/*! MD5 hash of downloaded file (empty if nothing was downloaded). */
	std::string				m_downloadedFileMD5;

signals:
	/*! Signals is emitted when checkForUpdateInfo() was called in non-interactive mode.
		\param res Result of html request: 0 - ok, 1 - no update info (or no update file for this platform), 2 - connection error.
		\param newVersion New version available for current platform.
	*/
	void updateInfoRetrieved(int res, QString newVersion);

	/*! Emitted, when update dialog was cancelled by user through "skip-update" button. */
	void updateRejected(QString rejectedVersion);

private slots:
	/*! Triggered by network reply when download of update info has finished. */
	void downloadFinished(QNetworkReply *reply);

private:
	/*! If true, a download attempt for update info is followed by either an error message box or a update-download dialog. */
	bool					m_interactive;
	/*! Change log text (should be plain text). */
	QString					m_changeLogText;
	/*! New version available on server for current platform. */
	QString					m_newVersion;
	/*! Current version to be compared against version reported from server. */
	QString					m_currentVersion;
	/*! Path to the download info directory. */
	QString					m_updateInfoDownloadPath;

	/*! Url to update-installer. */
	QString					m_downloadUrl;

	/*! Local file path to downloaded update installer. */
	QString					m_downloadFilePath;

	/*! Holds the version string of the programm version that was last rejected for update.
		For example, when the current program version is 2.0.15, and the version 2.0.16 is available, and the user has
		clicked on "Skip update", m_newestRejectedVersion will be set to 2.0.16.
		In the next update check, the program version and m_newestRejectedVersion will be compared and the newer one
		will be used in the check for "current version". Thus, in the next call to the software, 2.0.16 will be compared
		to the available 2.0.16 version and the update info won't be shown.
	*/
	QString					m_newestRejectedVersion;

	/*! Network manager, used to execute network requests. */
	QNetworkAccessManager	*m_networkManager;
};

} // namespace QtExt

#endif // QtExt_AutoUpdaterH
