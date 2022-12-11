#include "SVPluginLoader.h"

#include <IBK_messages.h>
#include <IBK_MultiLanguageString.h>
#include <IBK_Version.h>

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
	// check for matching versions before attempting to load the plugin
	if (pd.m_abiVersion.isEmpty() || !pd.matchesVersion()) {
		pd.m_result = LR_IncompatibleVersion;
		IBK::IBK_Message(IBK::FormatString("  Plugin library '%1' does not match installed version '%2', since it requires '%3'.\n")
						 .arg(pluginPath.toStdString()), IBK::MSG_ERROR);
		return;
	}

	QObject *plugin = pd.m_loader.instance();
	if (plugin != nullptr) {
		IBK::IBK_Message(IBK::FormatString("  Loading '%1'\n").arg(pluginFilePath.toStdString()) );
		// ok, we have a plugin, check for valid interfaces

		SVDatabasePluginInterface * dbIface = qobject_cast<SVDatabasePluginInterface*>(plugin);
		if (dbIface != nullptr) {
			pd.m_interfaceType = IT_Database;
			IBK::IBK_Message(IBK::FormatString("    Database interface\n") );
		}
		else {
			SVImportPluginInterface * iface = qobject_cast<SVImportPluginInterface*>(plugin);
			if (iface != nullptr) {
				pd.m_interfaceType = IT_Import;
				IBK::IBK_Message(IBK::FormatString("    Import interface\n") );
			}
		}
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

QString retrieveMultilanguageText(const QJsonObject & o, const QString & name) {
	if (o.contains(name) && o[name].isString()) {
		IBK::MultiLanguageString ml( o[name].toString().toStdString() );
		// retrieve string for current languange
		std::string langID = QtExt::LanguageHandler::instance().langId().toStdString();
		return QString::fromStdString(ml.string(langID, "en")); // use english as fall-back language
	}
	return QString();
}

void SVPluginLoader::PluginData::decodeMetadata() {
	m_longDesc = retrieveMultilanguageText(m_metadata, "long-description");
	m_shortDesc = retrieveMultilanguageText(m_metadata, "short-description");
	m_pluginVersion = m_metadata["version"].toString();
	m_abiVersion = m_metadata["abi-version"].toString();
	m_license = m_metadata["license"].toString();
	m_webpage = m_metadata["webpage"].toString();
	m_author = m_metadata["author"].toString();
}


bool SVPluginLoader::PluginData::matchesVersion() const {
	QStringList range = m_abiVersion.split("-");
	unsigned int major1=0, minor1=0;
	unsigned int major2=0, minor2=0;
	if (range.count() == 2) {
		if (!IBK::Version::extractMajorMinorVersionNumber(range[0].toStdString(), major1, minor1) &&
			!IBK::Version::extractMajorMinorVersionNumber(range[1].toStdString(), major2, minor2))
		{
			return false;
		}
	}
	else {
		if (!IBK::Version::extractMajorMinorVersionNumber(range[0].toStdString(), major1, minor1))
			return false;
		major2 = major1;
		minor2 = minor1;
	}
	// now check if vicus version is between first and second version
	IBK::Version first(major1, minor1);
	IBK::Version second(major2, minor2);
	IBK::Version vicusVer(VICUS::VERSION);
	if (first > vicusVer || second < vicusVer) return false;
	else return true;
}
