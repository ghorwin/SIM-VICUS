#include "SVPluginLoader.h"

#include <IBK_messages.h>

#include <QPluginLoader>

SVPluginLoader::SVPluginLoader()
{

}


void SVPluginLoader::loadPlugins() {
//	IBK::IBK_Message("Loading plugins...\n");
//	const auto staticInstances = QPluginLoader::staticInstances();
//	for (QObject *plugin : staticInstances) {
//		setupPluginMenuEntries(plugin);
//	}

//	QDir pluginsDir(SVSettings::instance().m_installDir + "/plugins");
//#if defined(Q_OS_WIN32)
//	SetDllDirectoryW(pluginsDir.absolutePath().toStdWString().c_str());
//#endif
//	IBK::IBK_Message(IBK::FormatString("Loading plugins in directory '%1'\n").arg(pluginsDir.absolutePath().toStdString()) );

//	const auto entryList = pluginsDir.entryList(QDir::Files);
//	for (const QString &fileName : entryList) {
//		QString ext = QFileInfo(fileName).suffix();
//		if (ext != "so" && ext != "dll" && ext != "dylib")
//			continue;
//		// skip files that do not have a valid file extensions
//		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
//		loader.setLoadHints(QLibrary::DeepBindHint);  // when loading plugins with IBK library support (yet other versions), this ensures that libraries use their own statically linked code
//		QObject *plugin = loader.instance();
//		if (plugin != nullptr) {
//			IBK::IBK_Message(IBK::FormatString("  Loading '%1'\n").arg(IBK::Path(fileName.toStdString()).filename().withoutExtension()) );
//			setupPluginMenuEntries(plugin);
//		}
//		else {
//			QString errtxt = loader.errorString();
//			IBK::IBK_Message(IBK::FormatString("  Error loading plugin library '%1': %2\n")
//							 .arg(IBK::Path(fileName.toStdString()).filename().withoutExtension()).arg(errtxt.toStdString()),
//							 IBK::MSG_ERROR);
//		}
//	}
}
