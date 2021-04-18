#include "SVSettings.h"

#include <QApplication>
#include <QSettings>
#include <QScreen>
#include <QDebug>
#include <QProcess>
#include <QFontInfo>

#ifdef Q_OS_WIN
#undef UNICODE
#include <windows.h>
#endif

#include <QtExt_Directories.h>

#include <tinyxml.h>

#include <VICUS_KeywordList.h>

#include "SVClimateDataTableModel.h"

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
	delete m_climateDataTableModel;
	m_climateDataTableModel = nullptr;
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

	// default to XTerm

	m_terminalEmulator = TE_XTerm;
//	m_monospaceFont = "Monospace";
//	// if we have "Bitstream Vera Sans Mono" or "Cousine" we use that instead
//	QStringList fontCandidates = QStringList() << "Bitstream Vera Sans Mono" << "Cousine";
//	for (QString f : fontCandidates) {
//		QFont fo(f);
//		if (QFontInfo(fo).exactMatch()) {
//			m_monospaceFont = f;
//			break;
//		}
//	}

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

	m_fontPointSize = settings.value("FontPointSize", 0).toUInt();
	m_invertYMouseAxis = settings.value("InvertYMouseAxis", m_invertYMouseAxis).toBool();
	m_terminalEmulator = (TerminalEmulators)settings.value("TerminalEmulator", TE_XTerm).toInt();

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


	m_db.readDatabases();
}


void SVSettings::write(QByteArray geometry, QByteArray state) {
	QtExt::Settings::write(geometry, state);

	QSettings settings( m_organization, m_appName );
	settings.setValue("VisibleDockWidgets", m_visibleDockWidgets.join(","));
	settings.setValue("PostProcExecutable", m_postProcExecutable );
	settings.setValue("CCMEditorExecutable", m_CCMEditorExecutable );
	settings.setValue("FontPointSize", m_fontPointSize);
	settings.setValue("InvertYMouseAxis", m_invertYMouseAxis);
	settings.setValue("TerminalEmulator", m_terminalEmulator);

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

	m_db.writeDatabases();
}


void SVSettings::readMainWindowSettings(QByteArray &geometry, QByteArray &state) {
	QtExt::Settings::readMainWindowSettings(geometry, state);

	QSettings settings( m_organization, m_appName );
	QString defaultDockWidgets = "Materials,Log";
	m_visibleDockWidgets = settings.value("VisibleDockWidgets", defaultDockWidgets).toString().split(",");
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


bool SVSettings::startProcess(const QString & executable,
									QStringList commandLineArgs,
									const QString & projectFile,
									TerminalEmulators terminalEmulator)
{
	bool success;
	// spawn process
#ifdef Q_OS_WIN

	/// \todo use wide-string version of API and/or encapsulate spawn process into a function

	// Use WinAPI to create a solver process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	std::string utf8String = projectFile.toStdString().data();
	si.lpTitle = (LPSTR)utf8String.c_str();
//	si.dwFlags = STARTF_USESHOWWINDOW;
//	si.wShowWindow = SW_SHOW;
	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;
	QString cmdLine = QString("\"%1\" %2 \"%3\"")
		.arg(executable)
		.arg(commandLineArgs.join(" "))
		.arg(projectFile);

	std::string cmd = cmdLine.toLatin1().data();
	// Start the child process.
	if( !CreateProcess( NULL,   // No module name (use command line).
		&cmd[0], 				// Command line.
		NULL,             		// Process handle not inheritable.
		NULL,             		// Thread handle not inheritable.
		FALSE,            		// Set handle inheritance to FALSE.
		lower_priority,   		// Create with priority lower then normal.
		NULL,             		// Use parent's environment block.
		NULL,             		// Use parent's starting directory.
		&si,              		// Pointer to STARTUPINFO structure.
		&pi )             		// Pointer to PROCESS_INFORMATION structure.
	)
	{
		return false;
	}
	return true;

#else // Q_OS_WIN

	// append project file to arguments, no quotes needed, since Qt takes care of that
	commandLineArgs << projectFile;
	qint64 pid;
	switch (terminalEmulator) {
		case TE_XTerm : {
			commandLineArgs = QStringList() << "-hold"
											<< "-fa" << "'Monospace'"
											<< "-fs" << "9"
											<< "-geometry" << "120x40" << "-e" << executable << commandLineArgs;
			QString terminalProgram = "xterm";
			success = QProcess::startDetached(terminalProgram, commandLineArgs, QString(), &pid);
		} break;

		case TE_GnomeTerminal : {
			commandLineArgs = QStringList() << "--tab"  << "--" << "/bin/bash" << "'" + commandLineArgs.join(" ") + "'";
			QString terminalProgram = "gnome-terminal";
			success = QProcess::startDetached(terminalProgram, commandLineArgs, QString(), &pid);
		} break;

		default:
			success = QProcess::startDetached(executable, commandLineArgs, QString(), &pid);
	}


	// TODO : do something with the process identifier... mayby check after a few seconds, if the process is still running?
	return success;

#endif // Q_OS_WIN
}


QString SVSettings::nandradSolverExecutable() {
	QString solverExecutable = QFileInfo(SVSettings::instance().m_installDir + "/NandradSolver").filePath();
#ifdef WIN32
	solverExecutable += ".exe";
#endif // WIN32
	return solverExecutable;
}

QString SVSettings::view3dExecutable() {
	QString solverExecutable = QFileInfo(SVSettings::instance().m_installDir + "/View3D").filePath();
#ifdef WIN32
	solverExecutable += ".exe";
#endif // WIN32
	return solverExecutable;
}


SVClimateDataTableModel * SVSettings::climateDataTableModel() {
	if (m_climateDataTableModel == nullptr) {
		m_climateDataTableModel = new SVClimateDataTableModel(nullptr); // we delete the object in the destructor
		m_climateDataTableModel->updateClimateFileList();
	}
	return m_climateDataTableModel;
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
			m_majorGridColor = QColor("#9793a0");
			m_minorGridColor = QColor("#27272c");
			m_sceneBackgroundColor = QColor("#13141a");
			m_selectedSurfaceColor = QColor("#3465a4");
		break;
		case NUM_TT: ; // just to make compiler happy
	}
}
