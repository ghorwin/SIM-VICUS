#ifndef SVPluginLoaderH
#define SVPluginLoaderH

#include <QList>
#include <IBK_MultiLanguageString.h>


/*! Implements all plugin loading/updating logic and stores collected information
	about available and installed plugins for use by the plugin manager.
*/
class SVPluginLoader {
public:

	struct PluginData {
		/*! Unique Plugin-Name, i.e.  */
		QString m_pluginPath;
		/*! If true, the plugin is read from built-in plugin directory. */
		bool	m_builtIn;

		IBK::MultiLanguageString m_description;
		IBK::MultiLanguageString m_longText;
	};

	SVPluginLoader();

	void loadPlugins();

	/*! Stores data about all collected plugins. */
	QList<PluginData>	m_plugins;

};

#endif // SVPluginLoaderH
