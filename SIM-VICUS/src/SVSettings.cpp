#include "SVSettings.h"

#include <QApplication>
#include <QSettings>
#include <QScreen>
#include <QDebug>

#include <QtExt_Directories.h>
#include <tinyxml.h>

#include <VICUS_KeywordList.h>

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
			   typename std::map<unsigned int, T> & db)
{
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( topLevelTag );
	doc.LinkEndChild(root);

	for (auto e : db)
		e.second.writeXML(root);

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

	// initialize theme settings
	m_themeSettings[TT_White].setDefaults(TT_White);
	m_themeSettings[TT_Dark].setDefaults(TT_Dark);

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

	m_invertYMouseAxis = settings.value("InvertYMouseAxis", m_invertYMouseAxis).toBool();

	SVSettings::ThemeType tmpTheme = (SVSettings::ThemeType)settings.value("Theme", m_theme ).toInt();
	m_theme = tmpTheme;
	// read theme-specific settings
	settings.beginGroup("DarkThemeSettings");
	m_themeSettings[TT_Dark].m_majorGridColor = settings.value("MajorGridColor", m_themeSettings[TT_Dark].m_majorGridColor).value<QColor>();
	m_themeSettings[TT_Dark].m_minorGridColor = settings.value("MinorGridColor", m_themeSettings[TT_Dark].m_minorGridColor).value<QColor>();
	m_themeSettings[TT_Dark].m_sceneBackgroundColor = settings.value("SceneBackgroundColor", m_themeSettings[TT_Dark].m_sceneBackgroundColor).value<QColor>();
	m_themeSettings[TT_Dark].m_selectedSurfaceColor = settings.value("SelectedSurfaceColor", m_themeSettings[TT_Dark].m_selectedSurfaceColor).value<QColor>();
	settings.endGroup();
	settings.beginGroup("BrightThemeSettings");
	m_themeSettings[TT_White].m_majorGridColor = settings.value("MajorGridColor", m_themeSettings[TT_White].m_majorGridColor).value<QColor>();
	m_themeSettings[TT_White].m_minorGridColor = settings.value("MinorGridColor", m_themeSettings[TT_White].m_minorGridColor).value<QColor>();
	m_themeSettings[TT_White].m_sceneBackgroundColor = settings.value("SceneBackgroundColor", m_themeSettings[TT_White].m_sceneBackgroundColor).value<QColor>();
	m_themeSettings[TT_White].m_selectedSurfaceColor = settings.value("SelectedSurfaceColor", m_themeSettings[TT_White].m_selectedSurfaceColor).value<QColor>();
	settings.endGroup();
//		qDebug() << m_themeSettings[TT_White].m_majorGridColor.name()
//				 << m_themeSettings[TT_White].m_minorGridColor.name()
//				 << m_themeSettings[TT_White].m_sceneBackgroundColor.name()
//				 << m_themeSettings[TT_White].m_selectedSurfaceColor.name();

	m_fontPointSize = settings.value("FontPointSize", 0).toUInt();
}


void SVSettings::write(QByteArray geometry, QByteArray state) {
	QtExt::Settings::write(geometry, state);

	QSettings settings( m_organization, m_appName );
	settings.setValue("VisibleDockWidgets", m_visibleDockWidgets.join(","));
	settings.setValue("PostProcExecutable", m_postProcExecutable );
	settings.setValue("CCMEditorExecutable", m_CCMEditorExecutable );
	settings.setValue("FontPointSize", m_fontPointSize);
	settings.setValue("InvertYMouseAxis", m_invertYMouseAxis);
	settings.setValue("Theme", m_theme);

	// write theme-specific settings
	settings.beginGroup("DarkThemeSettings");
	settings.setValue("MajorGridColor", m_themeSettings[TT_Dark].m_majorGridColor);
	settings.setValue("MinorGridColor", m_themeSettings[TT_Dark].m_minorGridColor);
	settings.setValue("SceneBackgroundColor", m_themeSettings[TT_Dark].m_sceneBackgroundColor);
	settings.setValue("SelectedSurfaceColor", m_themeSettings[TT_Dark].m_selectedSurfaceColor);
	settings.endGroup();

	settings.beginGroup("BrightThemeSettings");
	settings.setValue("MajorGridColor", m_themeSettings[TT_White].m_majorGridColor);
	settings.setValue("MinorGridColor", m_themeSettings[TT_White].m_minorGridColor);
	settings.setValue("SceneBackgroundColor", m_themeSettings[TT_White].m_sceneBackgroundColor);
	settings.setValue("SelectedSurfaceColor", m_themeSettings[TT_White].m_selectedSurfaceColor);
	settings.endGroup();

	for (QMap<PropertyType, QVariant>::const_iterator it = m_propertyMap.constBegin();
		 it != m_propertyMap.constEnd(); ++it)
	{
		settings.setValue(PROPERTY_KEYWORDS[it.key()], it.value());
	}

	writeDatabase();
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

	readXMLDB(dbDir / "db_materials.xml", "Materials", "Material", m_dbOpaqueMaterials, true);
	readXMLDB(dbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", m_dbWindowGlazingSystems, true);
	readXMLDB(dbDir / "db_windows.xml", "Windows", "Window", m_dbWindows, true);
	readXMLDB(dbDir / "db_constructions.xml", "Constructions", "Construction", m_dbConstructions, true);
	readXMLDB(dbDir / "db_surfaceProperties.xml", "SurfaceProperties", "SurfaceProperty", m_dbSurfaceProperty, true);
	readXMLDB(dbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", m_dbBoundaryCondition, true);
	readXMLDB(dbDir / "db_pipes.xml", "Pipes", "Pipe", m_dbPipes, true);
	readXMLDB(dbDir / "db_fluids.xml", "Fluids", "Fluid", m_dbFluids, true);

	// user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	readXMLDB(userDbDir / "db_materials.xml", "Materials", "Material", m_dbOpaqueMaterials);
	readXMLDB(userDbDir / "db_windows.xml", "Windows", "Window", m_dbWindows);
	readXMLDB(userDbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", m_dbWindowGlazingSystems);
	readXMLDB(userDbDir / "db_constructions.xml", "Constructions", "Construction", m_dbConstructions);
	readXMLDB(userDbDir / "db_surfaceProperties.xml", "SurfaceProperties", "SurfaceProperty", m_dbSurfaceProperty);
	readXMLDB(userDbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", m_dbBoundaryCondition);
	readXMLDB(userDbDir / "db_pipes.xml", "Pipes", "Pipe", m_dbPipes);
	readXMLDB(userDbDir / "db_fluids.xml", "Fluids", "Fluid", m_dbFluids);
}


void SVSettings::writeDatabase() {

	// we only write user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

#if 0
	// create some dummy materials to write out
	VICUS::Material m;
	m.m_id = 100000;
	m.m_category = VICUS::Material::MC_Bricks;
	m.m_dataSource = "SimQuality";
	m.m_manufacturer = "generic";
	m.m_color = "#800000";
	m.m_notes = "en:Massiv contrete-type material used in SimQuality test cases.|de:Massives, Beton-ähnliches Material zur Verwendung in SimQuality.";
	m.m_displayName = "en:Concrete|de:Beton";
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Density, 2000);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_HeatCapacity, 1000);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Conductivity, 1.2);

	m_dbOpaqueMaterials[m.m_id] = m;
#endif

	writeXMLDB(userDbDir / "db_materials.xml", "Materials", m_dbOpaqueMaterials);
	writeXMLDB(userDbDir / "db_windows.xml", "Windows", m_dbWindowGlazingSystems);
	writeXMLDB(userDbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", m_dbWindowGlazingSystems);
	writeXMLDB(userDbDir / "db_constructions.xml", "Constructions", m_dbConstructions);
	writeXMLDB(userDbDir / "db_surfaceProperties.xml", "SurfaceProperties", m_dbSurfaceProperty);
	writeXMLDB(userDbDir / "db_boundaryConditions.xml", "BoundaryConditions", m_dbBoundaryCondition);
	writeXMLDB(userDbDir / "db_pipes.xml", "Pipes", m_dbPipes);
	writeXMLDB(userDbDir / "db_fluids.xml", "Fluids", m_dbFluids);
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


void SVSettings::ThemeSettings::setDefaults(SVSettings::ThemeType theme) {
	switch (theme) {
		case TT_White :
			m_majorGridColor = QColor("#202020");
			m_minorGridColor = QColor("#d3d7cf");
			m_sceneBackgroundColor = QColor("#fffcf5");
			m_selectedSurfaceColor = QColor("#729fcf");
		break;

		case TT_Dark :
			m_majorGridColor = QColor("#7174a0");
			m_minorGridColor = QColor("#46455d");
			m_sceneBackgroundColor = QColor("#010c1f");
			m_selectedSurfaceColor = QColor("#3465a4");
		break;
		case NUM_TT: ; // just to make compiler happy
	}
}
