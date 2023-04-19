#include "SVPluginLoader.h"

#include <memory>

#include <IBK_messages.h>
#include <IBK_MultiLanguageString.h>
#include <IBK_Version.h>

#include <QPluginLoader>
#include <QCoreApplication>
#include <QDir>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <QtExt_LanguageHandler.h>
#include <QtExt_Directories.h>

#include "SVSettings.h"

#include "plugins/SVDatabasePluginInterface.h"
#include "plugins/SVImportPluginInterface.h"

SVPluginLoader::SVPluginLoader()
{

}


void SVPluginLoader::loadPlugins() {
	IBK::IBK_Message("Loading plugins...\n");

	// first load built-in (installed) plugins
	QDir pluginsDir(SVSettings::instance().m_installDir + "/plugins");
	IBK::IBK_Message(IBK::FormatString("Loading plugins in directory '%1'\n").arg(pluginsDir.absolutePath().toStdString()) );

	QStringList entryList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (const QString &dirName : entryList) {
		// attempt to load a plugin in this directory
		PluginData pd;
		loadPlugin(pluginsDir.filePath(dirName), pd);
	}


	// then load user-installed plugins
	pluginsDir = QDir(QtExt::Directories::userDataDir() + "/plugins");
	IBK::IBK_Message(IBK::FormatString("Loading plugins in directory '%1'\n").arg(pluginsDir.absolutePath().toStdString()) );

	entryList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (const QString &dirName : entryList) {
		// attempt to load a plugin in this directory
		PluginData pd;
		loadPlugin(pluginsDir.filePath(dirName), pd);
	}

//	const auto staticInstances = QPluginLoader::staticInstances();
//	for (QObject *plugin : staticInstances) {
//		setupPluginMenuEntries(plugin);
//	}

}


void SVPluginLoader::loadPlugin(const QString & pluginPath, PluginData & pd) {
	FUNCID(SVPluginLoader::loadPlugin);

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
			if (count == 0)
				pluginFile = fileName;
			++count;
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
	pd.m_loader->setFileName(pluginFilePath);
	pd.m_loader->setLoadHints(QLibrary::DeepBindHint);  // when loading plugins with IBK library support (yet other versions), this ensures that libraries use their own statically linked code
	pd.m_metadata = pd.m_loader->metaData().value("MetaData").toObject();
	pd.decodeMetadata(); // to have some fallback data
	// check for matching versions before attempting to load the plugin
	// Plugin must have the same version as SIM-VICUS!
	if (pd.m_abiVersion.isEmpty() || !pd.matchesVersion()) {
		pd.m_result = LR_IncompatibleVersion;
		IBK::IBK_Message(IBK::FormatString("  Plugin library '%1' does not match installed version '%2', since it requires '%3'.\n")
						 .arg(pluginPath.toStdString()).arg(VICUS::VERSION).arg(pd.m_abiVersion.toStdString()), IBK::MSG_ERROR);
		return;
	}

	QObject * plugin = nullptr;
	QDir::setCurrent(pluginDir.absolutePath());
	// add path to plugin to library search paths
	qApp->addLibraryPath(pluginDir.absolutePath());
	for (const QString &path : qApp->libraryPaths() )
		qDebug() << path;
	bool success = pd.m_loader->load(); // load the plugin
	// and remove the path again, so that subsequent libs won't load wrong dll/so-files
	qApp->removeLibraryPath(pluginDir.absolutePath());
	if (success)
		plugin = pd.m_loader->instance(); // access the library

	// error handling
	if (!success || plugin == nullptr) {
		QString errtxt = pd.m_loader->errorString();
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
	else {
		IBK::MessageIndentor ident2;
		IBK::IBK_Message(IBK::FormatString("  Loading '%1'\n").arg(pluginFilePath.toStdString()) );
		// ok, we have a plugin, check for valid interfaces

		// Mind: use qobject_cast here!
		SVDatabasePluginInterface * dbIface = qobject_cast<SVDatabasePluginInterface*>(plugin);
		if (dbIface != nullptr) {
			pd.m_interfaceType = IT_Database;
			dbIface->setLanguage(QtExt::LanguageHandler::langId(), QtExt::Directories::appname);
			IBK::MessageIndentor ident;
			IBK::IBK_Message(IBK::FormatString("%1, Version %2, Database interface\n").arg(dbIface->title().toStdString()).arg(pd.m_pluginVersion.toStdString()) );
			m_plugins[pluginFile] = pd;
		}
		else {
			// Mind: use qobject_cast here!
			SVImportPluginInterface * iface = qobject_cast<SVImportPluginInterface*>(plugin);
			if (iface != nullptr) {
				pd.m_interfaceType = IT_Import;
				iface->setLanguage(QtExt::LanguageHandler::langId(), QtExt::Directories::appname);
				IBK::MessageIndentor ident;
				IBK::IBK_Message(IBK::FormatString("%1, Version %2, Import interface\n").arg(iface->title().toStdString()).arg(pd.m_pluginVersion.toStdString()) );
				m_plugins[pluginFile] = pd;
			}
			else {
				// not one of our interfaces
				IBK::IBK_Message("Not a recognized interface, ignored.\n", IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD );
			}
		}
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
	m_title = retrieveMultilanguageText(m_metadata, "title");
	m_longDesc = retrieveMultilanguageText(m_metadata, "long-description");
	m_shortDesc = retrieveMultilanguageText(m_metadata, "short-description");
	m_pluginVersion = m_metadata["version"].toString();
	m_abiVersion = m_metadata["vicus-version"].toString();
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
