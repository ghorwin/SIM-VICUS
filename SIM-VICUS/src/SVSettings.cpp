#include "SVSettings.h"

#include <QApplication>
#include <QSettings>
#include <QScreen>

#include <QtExt_Directories.h>
#include <tinyxml.h>

SVSettings * SVSettings::m_self = nullptr;

const char * const SVSettings::PROPERTY_KEYWORDS[SVSettings::NUM_PT] = {
	"LastFileOpenDirectory",
	"UseModernSolver",
	"ClearResultDirBeforeStart",
	"NumParallelThreads",
};


SVSettings & SVSettings::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVSettings::instance]", "You must create an instance of "
		"SVSettings before accessing SVSettings::instance()!");
	return *m_self;
}

template<typename T>
void readXMLDB(const IBK::Path & fname, const std::string & topLevelTag,
			   const std::string & childTagName,
			   typename std::map<unsigned int, T> & db,
			   bool builtIn = false)
{
	FUNCID(SVSettings-readXMLDB);

	TiXmlDocument doc;
	std::map<std::string,IBK::Path> pathPlaceHolders; // only dummy for now, filenamePath does not contain placeholders
	if (!fname.isFile() )
		return;

	if (!doc.LoadFile(fname.str().c_str(), TIXML_ENCODING_UTF8)) {
		throw IBK::Exception(IBK::FormatString("Error in line %1 of project file '%2':\n%3")
				.arg(doc.ErrorRow())
				.arg(fname)
				.arg(doc.ErrorDesc()), FUNC_ID);
	}

	// we use a handle so that NULL pointer checks are done during the query functions
	TiXmlHandle xmlHandleDoc(&doc);

	// read root element
	TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
	if (!xmlElem)
		return; // empty file?
	std::string rootnode = xmlElem->Value();
	if (rootnode != topLevelTag)
		throw IBK::Exception( IBK::FormatString("Expected '%1' as root node in XML file.")
							  .arg(topLevelTag), FUNC_ID);

	try {
		const TiXmlElement * c2 = xmlElem->FirstChildElement();
		while (c2) {
			const std::string & c2Name = c2->ValueStr();
			if (c2Name != childTagName)
				IBK::IBK_Message(IBK::FormatString("Unknown/unsupported tag '%1' in line %2, expected '%3'.")
								 .arg(c2Name).arg(c2->Row()).arg(childTagName), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			T obj;
			obj.readXML(c2);
			obj.m_builtIn = builtIn;

			// \todo check for existing ID and issue warning/error
			if(db.find(obj.m_id) != db.end()){
				IBK::Exception(IBK::FormatString("Database '%1' contains duplicate ids %2 ")
							   .arg(fname.str()).arg(obj.m_id), FUNC_ID);
			}

			db[obj.m_id] = obj;
			c2 = c2->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading XML database '%1'.").arg(fname), FUNC_ID);
	}

}


template<typename T>
void writeXMLDB(const IBK::Path & fname, const std::string & topLevelTag,
			   typename std::map<unsigned int, T> & db,
			   bool builtIn = false)
{
	FUNCID(SVSettings-writeXMLDB);

	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( topLevelTag );
	doc.LinkEndChild(root);


	for (auto e : db) {
		if(builtIn == e.second.m_builtIn)
			e.second.writeXML(root);
	}

	doc.SaveFile( fname.c_str() );


}


SVSettings::SVSettings(const QString & organization, const QString & appName) :
	QtExt::Settings(organization, appName)
{
	// singleton check
	Q_ASSERT_X(m_self == nullptr, "[SVSettings::SVSettings]", "You must not create multiple instances of "
		"classes that derive from SVSettings!");
	for(unsigned int i=0; i<NumCmdLineFlags; ++i)
		m_flags[i] = false;
	m_self = this;
}


SVSettings::~SVSettings() {
	m_self = nullptr;
}


void SVSettings::setDefaults() {
	QtExt::Settings::setDefaults();

	m_propertyMap[PT_ClearResultDirBeforeStart] = true;

#ifdef Q_OS_WIN
	m_7zExecutable = "7z.exe";
#else
	m_7zExecutable = "7z";
#endif
	m_thumbNailSize = 400;

	m_fontPointSize = 0; // means: use auto-detected

	// initialize random number generator
	qsrand(time(nullptr));
}


void SVSettings::read() {
	QtExt::Settings::read();

	QSettings settings( m_organization, m_appName );

	for (unsigned int i=0; i<NUM_PT; ++i) {
		QVariant var = settings.value(PROPERTY_KEYWORDS[i], QVariant());
		if (var.isValid())
			m_propertyMap.insert((PropertyType)i, var);
	}

	QString tmpPostProcExecutable = settings.value("PostProcExecutable", m_postProcExecutable ).toString();
	if (!tmpPostProcExecutable.isEmpty())
		m_postProcExecutable = tmpPostProcExecutable;
	else {
		// auto-detect postproc 2 in install directory
#if defined(Q_OS_WIN)
		QString postProc2FilePath = m_installDir + "\\PostProcApp.exe";
#elif defined (Q_OS_MAC)
		QString postProc2FilePath = m_installDir + "/PostProcApp.app/Contents/MacOS/PostProcApp";
#else
		QString postProc2FilePath = m_installDir + "/PostProcApp";
#endif
		if (QFile(postProc2FilePath).exists())
			m_postProcExecutable = postProc2FilePath;
	}

	QString tmpCCMExecutable = settings.value("CCMEditorExecutable", m_CCMEditorExecutable ).toString();
	if (!tmpCCMExecutable.isEmpty())
		m_CCMEditorExecutable = tmpCCMExecutable;
	else {
		// auto-detect postproc 2 in install directory
#if defined(Q_OS_WIN)
		tmpCCMExecutable = m_installDir + "\\CCMEditor.exe";
#elif defined (Q_OS_MAC)
		tmpCCMExecutable = m_installDir + "/CCMEditor.app/Contents/MacOS/CCMEditorApp";
#else
		tmpCCMExecutable = m_installDir + "/CCMEditor";
#endif
		if (QFile(tmpCCMExecutable).exists())
			m_CCMEditorExecutable = tmpCCMExecutable;
	}

	SVSettings::ThemeType tmpTheme = (SVSettings::ThemeType)settings.value("Theme", m_theme ).toInt();
	m_theme = tmpTheme;

	m_invertYMouseAxis = settings.value("IntertYMouseAxis", m_invertYMouseAxis).toBool();

	m_fontPointSize = settings.value("FontPointSize", 0).toUInt();
}


void SVSettings::write(QByteArray geometry, QByteArray state) {
	QtExt::Settings::write(geometry, state);

	QSettings settings( m_organization, m_appName );
	settings.setValue("VisibleDockWidgets", m_visibleDockWidgets.join(","));
	settings.setValue("PostProcExecutable", m_postProcExecutable );
	settings.setValue("CCMEditorExecutable", m_CCMEditorExecutable );
	settings.setValue("FontPointSize", m_fontPointSize);
	settings.setValue("Theme", m_theme);
	settings.setValue("IntertYMouseAxis", m_invertYMouseAxis);

	for (QMap<PropertyType, QVariant>::const_iterator it = m_propertyMap.constBegin();
		 it != m_propertyMap.constEnd(); ++it)
	{
		settings.setValue(PROPERTY_KEYWORDS[it.key()], it.value());
	}
}


void SVSettings::readMainWindowSettings(QByteArray &geometry, QByteArray &state) {
	QtExt::Settings::readMainWindowSettings(geometry, state);

	QSettings settings( m_organization, m_appName );
	QString defaultDockWidgets = "Materials,Log";
	m_visibleDockWidgets = settings.value("VisibleDockWidgets", defaultDockWidgets).toString().split(",");
}


void SVSettings::readDatabase() {
	// built-in databases

	IBK::Path dbDir(QtExt::Directories::databasesDir().toStdString());

	readXMLDB(dbDir / "DB_Materials.xml", "Materials", "Material", m_dbOpaqueMaterials, true);
	readXMLDB(dbDir / "DB_WindowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", m_dbWindowGlazingSystems, true);
	readXMLDB(dbDir / "DB_Windows.xml", "Windows", "Window", m_dbWindows, true);
	readXMLDB(dbDir / "DB_Constructions.xml", "Constructions", "Construction", m_dbConstructions, true);
	readXMLDB(dbDir / "DB_SurfaceProperties.xml", "SurfaceProperties", "SurfaceProperty", m_dbSurfaceProperty, true);
	readXMLDB(dbDir / "DB_BoundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", m_dbBoundaryCondition, true);

	// user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	readXMLDB(userDbDir / "DB_Materials.xml", "Materials", "Material", m_dbOpaqueMaterials);
	readXMLDB(dbDir / "DB_Windows.xml", "Windows", "Window", m_dbWindows);
	readXMLDB(dbDir / "DB_WindowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", m_dbWindowGlazingSystems);
	readXMLDB(dbDir / "DB_Constructions.xml", "Constructions", "Construction", m_dbConstructions);
	readXMLDB(dbDir / "DB_SurfaceProperties.xml", "SurfaceProperties", "SurfaceProperty", m_dbSurfaceProperty);
	readXMLDB(dbDir / "DB_BoundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", m_dbBoundaryCondition);
}


void SVSettings::writeDatabase() {

	// we only write user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	writeXMLDB(userDbDir / "DB_Materials.xml", "Materials", m_dbOpaqueMaterials);
	writeXMLDB(userDbDir / "DB_Windows.xml", "Windows", m_dbWindowGlazingSystems);
	writeXMLDB(userDbDir / "DB_WindowGlazingSystems.xml", "WindowGlazingSystems", m_dbWindowGlazingSystems);
	writeXMLDB(userDbDir / "DB_Constructions.xml", "Constructions", m_dbConstructions);
	writeXMLDB(userDbDir / "DB_SurfaceProperties.xml", "SurfaceProperties", m_dbSurfaceProperty);
	writeXMLDB(userDbDir / "DB_BoundaryConditions.xml", "BoundaryConditions", m_dbBoundaryCondition);
}



void SVSettings::recursiveSearch(QDir baseDir, QStringList & files, const QStringList & extensions) {
	QStringList	fileList = baseDir.entryList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);

	foreach (QString f, fileList) {
		QString fullPath = baseDir.absoluteFilePath(f);
		QFileInfo finfo(fullPath);
		if (finfo.isDir()) {
			recursiveSearch(QDir(fullPath), files, extensions);
		}
		else {
			bool found = false;
			foreach (QString ext, extensions) {
				if (finfo.suffix() == ext) {
					found = true;
					break;
				}
			}
			if (found)
				files.append(fullPath);
		}
	}
}


unsigned int SVSettings::defaultApplicationFontSize() {
	QScreen *srn = QApplication::screens().at(0);
	qreal dotsPerInch = (qreal)srn->logicalDotsPerInch();
	qreal pixHeight = (qreal)srn->size().height();
	qreal hdp = pixHeight/dotsPerInch;
	QFont f = qApp->font();
	qreal ps = f.pointSizeF();
//	QFontMetrics fm(f);
//	qreal fontHeight = fm.lineSpacing();


	// *** Font size adjustment ***
#if defined(Q_OS_MAC)
	ps = 9*hdp/9.0;
	f.setPointSizeF(ps);
//	qApp->setDesktopSettingsAware(false);
#elif defined(Q_OS_UNIX)
//	qDebug() << f;
	ps = 9*hdp/9.0;
//	qApp->setDesktopSettingsAware(false);

#elif defined(Q_OS_WIN)
	ps = 8*hdp/9.0;
//	qApp->setDesktopSettingsAware(false);
#endif

	return (unsigned int)ps;
}
