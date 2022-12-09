#ifndef SVPluginLoaderH
#define SVPluginLoaderH

#include <QMap>
#include <QPixmap>
#include <QJsonObject>
#include <QPluginLoader>

/*! Implements all plugin loading/updating logic and stores collected information
	about available and installed plugins for use by the plugin manager.
*/
class SVPluginLoader {
public:

	enum LoadResult {
		/*! Used when plugin was read successfully and is working for this version of SIM-VICUS. */
		LR_Success,
		/*! Used when plugin is ok, but metadata (icon, text, description) is missing. */
		LR_IncompleteMetadata,
		/*! Used when plugin requires a different version of SIM-VICUS/VICUS ABI and cannot be used. */
		LR_IncompatibleVersion,
		/*! Used when plugin directory does not contain a valid/supported binary file. */
		LR_NoBinary,
		/*! Used when plugin could not be loaded due to missing dependencies. */
		LR_FailedToLoad
	};

	struct PluginData {
		/*! Attempts to retrieve as much useful information from the meta data in member m_metadata as possible. */
		void decodeMetadata();

		/*! Source-path, where Plugin-Shared-Lib was read from.
			May be of format "repository:<plugin-name>", in which case the plugin
			is retrieved upon installing from the webpage repository.
		*/
		QString m_pluginPath;
		/*! If true, the plugin is read from built-in plugin directory. */
		bool	m_builtIn;

		/*! The pluginloader (library wrapper) of this library.
			Only set to an object if library was successfully loaded.
		*/
		QPluginLoader	m_loader;

		/*! Stores the result of the load operation. */
		LoadResult	m_result;

		/*! Encoded metadata, either from file or delivered from plugin.
			Use decodeMetadata() to extract relevant information and store that in
			member variables for easy access.
		*/
		QJsonObject m_metadata;

		/*! A short description text, already in application language. */
		QString m_shortDesc;
		/*! A longer description text, already in application language. */
		QString m_longDesc;

		/*! The application icon. */
		QPixmap			m_icon;
		/*! The application screenshots (optional). */
		QList<QPixmap>	m_screenshots;

	};

	SVPluginLoader();

	void loadPlugins();

	/*! Stores data about all collected plugins.
		Key is unique plugin file path (of binary file) or url-prefix.
	*/
	QMap<QString, PluginData>	m_plugins;

private:

	/*! Attempts to load a plugin in the given path. */
	void loadPlugin(const QString & pluginPath, PluginData & pd);

	/*! Looks up metadata file for plugin and reads it as JSonObject. */
	QJsonDocument loadPluginMetadata(const QString & pluginPath);
};

#endif // SVPluginLoaderH
