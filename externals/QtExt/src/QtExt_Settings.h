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

#ifndef QtExt_SettingsH
#define QtExt_SettingsH

#include <QCoreApplication>
#include <QStringList>
#include <QMap>
#include <QMessageBox>

#include <IBK_messages.h>

namespace IBK {
	class ArgParser;
}

namespace QtExt {

/*! This class provides basic settings functionality, including:
	* read and write method of settings
	* generic parse functions for cmdline
	Essentially, this is a wrapper around QSettings that defines
	often used properties and handles saving/restoring of these
	properties.

	You should inherit this settings object in your own settings class,
	which can be a singleton.
*/
class Settings {
	Q_DECLARE_TR_FUNCTIONS(Settings)
	Q_DISABLE_COPY(Settings)
public:

	/*! Possible command line flags recognized by the program. */
	enum CmdLineFlags {
		/*! If set '--no-splash', the splash screen is not shown . */
		NoSplashScreen,
		/*! Number of flags. */
		NumCmdLineFlags
	};

	/*! Standard constructor.
		\param organization Some string defining the group/organization/company (major registry root name).
		\param appName Some string defining the application name (second part of registry root name).

		You may only instantiate one instance of the settings object in your application. An attempt to
		create a second instance will raise an exception in the constructor.
	*/
	Settings(const QString & organization, const QString & appName);

	/*! Destructor. */
	virtual ~Settings() {}

	/*! Sets default options (after first program start). */
	virtual void setDefaults();

	/*! Adds DSG-specific command line arguments to the arg parser. */
	virtual void updateArgParser(IBK::ArgParser & argParser);

	/*! Overrides DSG-content with command line arguments.
		This function is called after setDefaults() and read(). It can be used
		to override settings read from configuration or default settings.
	*/
	virtual void applyCommandLineArgs(const IBK::ArgParser & argParser);

	/*! Reads the user specific config data.
		The data is read in the usual method supported by the various platforms.
		The default implementation reads and populates all member variables.
	*/
	virtual void read();

	/*! Reads the main window configuration properties.
		\param geometry A bytearray with the main window geometry (see QMainWindow::saveGeometry())
		\param state A bytearray with the main window state (toolbars, dockwidgets etc.)
					(see QMainWindow::saveState())
	*/
	virtual void readMainWindowSettings(QByteArray &geometry, QByteArray &state);

	/*! Writes the user specific config data.
		The data is writted in the usual method supported by the various platforms.
		The default implementation writes all member variables.
		\param geometry A bytearray with the main window geometry (see QMainWindow::saveGeometry())
		\param state A bytearray with the main window state (toolbars, dockwidgets etc.)
					(see QMainWindow::saveState())
	*/
	virtual void write(QByteArray geometry, QByteArray state);

	/*! Convenience function, checks for defined text editor executable, than spawns an external
		process and shows the file in the text editor.
	*/
	bool openFileInTextEditor(QWidget * parent, const QString & filepath) const;

	/*! Shows a dialog with "Do not show again" check box. If the "Do not show" property is already set,
		this function does nothing.
		Use this function instead of creating a QMessageBox::information() dialog yourself.
		Pass a suitable unique dialog identification string for this dialog when calling this function.
	*/
	void showDoNotShowAgainMessage(QWidget * parent, const QString & doNotShowAgainDialogID,
										  const QString & title, const QString & msg);


	/*! Shows a dialog with "Do not show again" check box. If the "Do not show" property is already set,
		this function does nothing.
		Use this function instead of creating a QMessageBox::question() dialog yourself.
		Pass a suitable unique dialog identification string for this dialog when calling this function.
		Result is the pressed button. As default the Yes and No button will be shown.
		If the "Do not show" property is already set the last answer will be returned.
	*/
	QMessageBox::StandardButton showDoNotShowAgainQuestion(QWidget * parent, const QString & doNotShowAgainDialogID,
														   const QString & title, const QString & msg,
														   QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

	// ****** member variables ************

	/*! Organization ID name (used for QSettings).
		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_organization;

	/*! Application name (used for QSettings).
		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_appName;

	/*! Holds the file path to the project that
		should be opened during startup.

		By default, this value is set from m_lastProjectFile which is
		retrieved from the settings (i.e. registry) in read().
		However, if a project file was specified
		on the command line, the parseCmdLine() function will set
		this file instead.
		The separate property is needed in case that the command-line specified
		filename is invalid. In this case the m_lastProjectFile property is
		preserved and a start of the application without command line will
		correctly load the last successfully loaded project.

		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_initialProjectFile;

	/*! Installation path (without trailing backslash).
		This value is composed from the application file name in the setDefaults() function.
		\warning The install dir can be a relative path name.

		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_installDir;

	/*! Flags for command line settings. */
	bool						m_flags[NumCmdLineFlags];

	// *** Members below are stored in settings file ***


	/*! Maximum number of files in the MRU (most-recently-used) files list. */
	unsigned int				m_maxRecentProjects;

	/*! Entries of the MRU list. */
	QStringList					m_recentProjects;

	/*! Maximum number of undo steps. */
	unsigned int				m_maxNumUNDOSteps;

	/*! The file last opened in the UI.
		This value should be set by the application whenever a project
		file was successfully loaded or saved.
	*/
	QString						m_lastProjectFile;

	/*! Executable file name of external text editor.
		The text editor executable is determined automatically in setDefaults(),
		but can be customized by the user.
	*/
	QString						m_textEditorExecutable;

	/*! Current language ID (en, de, etc.)
		Can be interpreted by the application to load the respective translation files.
		This should be done before any UI component is instantiated.
		\note Language-ID might be empty, in which case the application should determine
			  the language ID automatically based on the current System locale.
	*/
	QString						m_langId;

	/*! If this is true, the native dialogs (Windows/Gnome/MacOS) for file and color dialogs
		will not be used, but the Qt-based dialog variants, instead.
	*/
	bool						m_dontUseNativeDialogs;

	/*! Stores the logging threshold for console. */
	IBK::verbosity_levels_t		m_userLogLevelConsole;

	/*! Stores the logging threshold for the log file. */
	IBK::verbosity_levels_t		m_userLogLevelLogfile;

	/*! The version number stored during last run (used to detect version changes). */
	QString						m_lastVersionNumber;

	/*! Map of do-not-show-again dialog IDs and their checked states. */
	QMap<QString, bool>			m_doNotShowAgainDialogs;

	/*! Map of do-not-show-again dialog IDs and their last answer.
		Only used for messages with result.
	*/
	QMap<QString, QMessageBox::StandardButton>	m_doNotShowAgainDialogsAnswer;

};

} // namespace QtExt

#endif // QtExt_SettingsH
