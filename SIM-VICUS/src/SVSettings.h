#ifndef SVSettingsH
#define SVSettingsH

#include <QtExt_Settings.h>

#include <QDir>
#include <QVariant>

class QDockWidget;

#include "SVDatabase.h"

/*! This class provides settings functionality, including:
	* read and write method of settings
	* generic parse functions for cmdline
	Essentially, this is a wrapper around QSettings that defines
	often used properties and handles saving/restoring of these
	properties.

	First thing in your application should be the instantiation
	of the Settings object.
	\code
	// initialize settings object
	SVSettings mySettings(organization, applicationName);
	// settings can be accessed application wide via the
	// singleton pattern
	SVSettings::instance().read();
	\endcode
*/
class SVSettings : public QtExt::Settings {
	Q_DECLARE_TR_FUNCTIONS(SVSettings)
	Q_DISABLE_COPY(SVSettings)
public:
	/*! Enumeration values for different properties to be managed in settings. */
	enum PropertyType {
		PT_LastFileOpenDirectory,
		PT_UseModernSolver,
		PT_ClearResultDirBeforeStart,
		PT_NumParallelThreads,
		NUM_PT
	};

	/*! Enumeration values for different themes to be managed in settings. */
	enum ThemeType {
		TT_White,
		TT_Dark,
		NUM_TT
	};

	/*! Keywords used for serialization of the properties. */
	static const char * const			PROPERTY_KEYWORDS[NUM_PT];

	/*! Returns the instance of the singleton. */
	static SVSettings & instance();


	// ****** public member functions *******

	/*! Standard constructor.
		\param organization Some string defining the group/organization/company (major registry root name).
		\param appName Some string defining the application name (second part of registry root name).

		You may only instantiate one instance of the settings object in your application. An attempt to
		create a second instance will raise an exception in the constructor.
	*/
	SVSettings(const QString & organization, const QString & appName);

	/*! Destructor. */
	~SVSettings();

	/*! Sets default options (after first program start). */
	void setDefaults();

	/*! Reads the user specific config data.
		The data is read in the usual method supported by the various platforms.
	*/
	void read();

	/*! Writes the user specific config data.
		The data is writted in the usual method supported by the various platforms.
		\param geometry A bytearray with the main window geometry (see QMainWindow::saveGeometry())
		\param state A bytearray with the main window state (toolbars, dockwidgets etc.)
					(see QMainWindow::saveState())
	*/
	void write(QByteArray geometry, QByteArray state);

	/*! Reads the main window configuration properties.
		\param geometry A bytearray with the main window geometry (see QMainWindow::saveGeometry())
		\param state A bytearray with the main window state (toolbars, dockwidgets etc.)
					(see QMainWindow::saveState())
	*/
	void readMainWindowSettings(QByteArray &geometry, QByteArray &state);

	/*! Convenience check function, tests if a property is in the map. */
	bool hasProperty(PropertyType t) const { return m_propertyMap.find(t) != m_propertyMap.end(); }

	// ****** static functions ************

	/*! Computes the default application font size based on screen properties. */
	static unsigned int defaultApplicationFontSize();

	/*! Recursive directory search function.
		Collects list of all files in directory 'baseDir' and all its child directories that
		match any of the extensions in stringlist 'extensions'.
		\todo Move to a different class, or make this a free function in QtExt
	*/
	static void recursiveSearch(QDir baseDir, QStringList & files, const QStringList & extensions);

	template <class T>
	static unsigned int firstFreeId(const std::map<unsigned int, T> &db, unsigned int id){
		for (;;++id) {
			if(db.find(id) == db.end())
				return id;
		}
		return id;
	}

	/*! Launches an external process (solver) in a console window. */
	static bool startProcess(const QString & executable, QStringList commandLineArgs, const QString & projectFile);

	// ****** member variables ************


	// *** members below are stored in settings file ***

	/*! Solver executable name. */
	QString						m_currentSolverExecutableName;

	/*! Path to external post processing. */
	QString						m_postProcExecutable;

	/*! Path to 7z executable. */
	QString						m_7zExecutable;

	/*! Path to CCMEditor executable. */
	QString						m_CCMEditorExecutable;

	/*! ThemeType of theme applied */
	ThemeType					m_theme = TT_Dark;

	/*! This struct stores the theme-specific settings that can be customized by the user. */
	struct ThemeSettings {
		void setDefaults(ThemeType theme);
		QColor						m_selectedSurfaceColor;
		QColor						m_majorGridColor;
		QColor						m_minorGridColor;
		QColor						m_sceneBackgroundColor;
	};

	/*! Container to store theme-specific settings. */
	ThemeSettings				m_themeSettings[NUM_TT];

	/*! If true, orbit controller uses inverted mouse axis. */
	bool						m_invertYMouseAxis = true;

	/*! The project file suffix including the . */
	QString						m_projectFileSuffix			= ".vicus";
	/*! The project package suffix including the . */
	QString						m_projectPackageSuffix		= ".vicpac";

	/*! Dimensions of the thumbnail to be generated for the welcome page. */
	unsigned int				m_thumbNailSize;

	/*! Base point size of application font.
		Overrides default, initialized with 0 to indicate (use-auto-detected).
		\sa defaultApplicationFontSize();
	*/
	unsigned int				m_fontPointSize;

	/*! Stores list of all visible dock widgets. */
	QStringList					m_visibleDockWidgets;

	/*! Generic property map to store and retrieve configuration settings. */
	QMap<PropertyType, QVariant>	m_propertyMap;

	/*! Sorted list of the top 10 frequently used quantities. */
	QList<QString>				m_frequentlyUsedQuantities;


	/*! Our database. */
	SVDatabase					m_db;


private:

	/*! The global pointer to the SVSettings object.
		This pointer is set in the constructor, and cleared in the destructor.
	*/
	static SVSettings			*m_self;

};


#endif // SVSettingsH
