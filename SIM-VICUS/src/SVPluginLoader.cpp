#include "SVPluginLoader.h"

#include <IBK_messages.h>
#include <IBK_MultiLanguageString.h>

#include <QPluginLoader>
#include <QDir>

#include <QtExt_LanguageHandler.h>

#include "SVSettings.h"
#include "plugins/SVDatabasePluginInterface.h"
#include "plugins/SVImportPluginInterface.h"

SVPluginLoader::SVPluginLoader()
{

}


void SVPluginLoader::loadPlugins() {
	IBK::IBK_Message("Loading plugins...\n");

//	const auto staticInstances = QPluginLoader::staticInstances();
//	for (QObject *plugin : staticInstances) {
//		setupPluginMenuEntries(plugin);
//	}

	// first load built-in (installed) plugins
	QDir pluginsDir(SVSettings::instance().m_installDir + "/plugins");
#if defined(Q_OS_WIN32)
	// Mind: we need a variable for the converted wstring here!!!
	std::wstring plugDir = pluginsDir.absolutePath().toStdWString();
	SetDllDirectoryW(plugDir.c_str());
#endif
	IBK::IBK_Message(IBK::FormatString("Loading plugins in directory '%1'\n").arg(pluginsDir.absolutePath().toStdString()) );

	const auto entryList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (const QString &dirName : entryList) {
		// attempt to load a plugin in this directory
		PluginData pd;
		loadPlugin(pluginsDir.filePath(dirName), pd);
	}

}


void SVPluginLoader::loadPlugin(const QString & pluginPath, PluginData & pd) {
	QDir pluginDir(pluginPath);
	const auto entryList = pluginDir.entryList(QDir::Files);
	pd.m_result = LR_NoBinary;
	QStringList libFiles;
	int count = 0;
	QString pluginFile;
	for (const QString &fileName : entryList) {
		QString ext = QFileInfo(fileName).suffix();
		// skip files that do not have a valid file extensions
		if (ext != "so" && ext != "dll" && ext != "dylib")
			continue;

		// one of the dll/so-files (NOTE: there may be several!) might be a plugin; try to load them all until we succeed.
		libFiles.append(fileName);
		// do we have a plugin file?
		if (fileName.contains("plugin", Qt::CaseInsensitive)) {
			++count;
			if (count == 0)
				pluginFile = fileName;
		}
	}

	if (libFiles.count() == 0) {
		pd.m_result = LR_NoBinary;
		IBK::IBK_Message(IBK::FormatString("  Plugin library path '%1' does not contains shared libs.\n")
						 .arg(pluginPath.toStdString()), IBK::MSG_ERROR);
		return;
	}
	// only one libFile? must be the plugin
	if (libFiles.count() == 1) {
		pluginFile = libFiles[0];
	}
	else {
		// if we have several lib files in directory, we must have one exactly one with "plugin" in the filename
		if (count > 1) {
			pd.m_result = LR_FailedToLoad;
			IBK::IBK_Message(IBK::FormatString("  Plugin library path '%1' contains several shared libs with 'plugin' filename part.\n")
							 .arg(pluginPath.toStdString()), IBK::MSG_ERROR);
			return;
		}
		else if (count == 0) {
			pd.m_result = LR_NoBinary;
			IBK::IBK_Message(IBK::FormatString("  Plugin library path '%1' does not contain a shared library with 'plugin' filename part.\n")
							 .arg(pluginPath.toStdString()), IBK::MSG_ERROR);
			return;
		}
		// pluginFile already holds filename to shared 'plugin' library
	}

	// try to load plugin
	QString pluginFilePath = pluginDir.absoluteFilePath(pluginFile);
	pd.m_loader.setFileName(pluginFilePath);
	pd.m_loader.setLoadHints(QLibrary::DeepBindHint);  // when loading plugins with IBK library support (yet other versions), this ensures that libraries use their own statically linked code
	pd.m_metadata = pd.m_loader.metaData().value("MetaData").toObject();
	pd.decodeMetadata(); // to have some fallback data
	// TODO : check for matching versions before attempting to load the plugin

	QObject *plugin = pd.m_loader.instance();
	if (plugin != nullptr) {
		IBK::IBK_Message(IBK::FormatString("  Loading '%1'\n").arg(pluginFilePath.toStdString()) );
		// ok, we have a plugin, retrieve metadata
	}
	else {
		QString errtxt = pd.m_loader.errorString();
		IBK::IBK_Message(IBK::FormatString("  Error loading plugin library '%1': %2\n")
						 .arg(pluginFilePath.toStdString()).arg(errtxt.toStdString()),
						 IBK::MSG_ERROR);

		pd.m_result = LR_FailedToLoad;
		QJsonDocument jdoc = loadPluginMetadata(pluginPath);
		if (jdoc.isNull())
			pd.m_metadata = QJsonObject();
		else
			pd.m_metadata = jdoc.object();
		pd.decodeMetadata(); // to have some fallback data
	}
}


QJsonDocument SVPluginLoader::loadPluginMetadata(const QString & pluginPath) {
	// compose plugin metadata.json file path and load it
	QString jsonFile = pluginPath + "/metafile.json";

	QFile loadFile(jsonFile);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		return QJsonDocument();
	}

	QByteArray metadata = loadFile.readAll();
	return QJsonDocument::fromJson(metadata);
}


void SVPluginLoader::PluginData::decodeMetadata() {
	if (m_metadata.contains("description") && m_metadata["description"].isString()) {
		IBK::MultiLanguageString ml( m_metadata["description"].toString().toStdString() );
		// retrieve string for current languange
		std::string langID = QtExt::LanguageHandler::instance().langId().toStdString();
		m_longDesc = QString::fromStdString(ml.string(langID, "en")); // use english as fall-back language
	}

	// TODO : other stuff

}
