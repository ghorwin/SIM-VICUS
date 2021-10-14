/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVSettings.h"

#include <QApplication>
#include <QSettings>
#include <QScreen>
#include <QDebug>
#include <QProcess>
#include <QFontInfo>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>

#ifdef Q_OS_WIN
#undef UNICODE
#include <Windows.h>
#endif

#include <QtExt_Directories.h>
#include <IBK_ArgParser.h>

#include <tinyxml.h>

#include <VICUS_KeywordList.h>

#include "SVClimateDataTableModel.h"

SVSettings * SVSettings::m_self = nullptr;

const char * const SVSettings::PROPERTY_KEYWORDS[SVSettings::NUM_PT] = {
	"LastFileOpenDirectory",
	"LastImportOpenDirectory",
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

	m_versionIdentifier = settings.value("VersionIdentifier", QString()).toString();
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
	settings.setValue("VersionIdentifier", m_versionIdentifier);
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


void SVSettings::updateArgParser(IBK::ArgParser & argParser) {
	QtExt::Settings::updateArgParser(argParser);
	argParser.addOption(0, "nandrad", "Generate NANDRAD simulation project from vicus-file.", "<target-name.nandrad>", "");
}


void SVSettings::applyCommandLineArgs(const IBK::ArgParser & argParser) {
	FUNCID(SVSettings::applyCommandLineArgs);
	QtExt::Settings::applyCommandLineArgs(argParser);

	if (argParser.hasOption("nandrad")) {
		// we need an initial project file argument
		if (m_initialProjectFile.isEmpty()) {
			IBK::IBK_Message("Incomplete command line, '--nandrad' option requires vicus project file argument.",
							 IBK::MSG_ERROR, FUNC_ID);
			exit(1);
		}
		std::string str = argParser.option("nandrad");
#ifdef Q_OS_WIN
		// On Windows, use codepage encoding instead of UTF8
		m_nandradExportFileName = QString::fromLatin1(str.c_str() );
#else
		m_nandradExportFileName = QString::fromUtf8( str.c_str() );
		// remove "file://" prefix
		if (m_nandradExportFileName.indexOf("file://") == 0)
			m_nandradExportFileName = m_nandradExportFileName.mid(7);
#endif
	}
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
			//  gnome-terminal -- /home/ghorwin/git/SIM-VICUS/data/vicus/Tutorial/run_in_gnome_terminal.sh  /home/ghorwin/git/SIM-VICUS/bin/release/NandradSolver /home/ghorwin/git/SIM-VICUS/data/vicus/Tutorial/Tutorial1.nandrad
			QString executablePath = QFileInfo(executable).dir().absolutePath();
			commandLineArgs = QStringList() << "--tab"  << "--" << executablePath + "/run_in_gnome_terminal.sh" << executable << commandLineArgs;

//			commandLineArgs = QStringList() << "--tab"  << "--" << "/bin/bash" << executable << "\"" + commandLineArgs.join(" ") + "\"";
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
#ifdef _WIN32
	solverExecutable += ".exe";
#endif // _WIN32
	return solverExecutable;
}

QString SVSettings::nandradFMUGeneratorExecutable() {
	QString solverExecutable = QFileInfo(SVSettings::instance().m_installDir + "/NandradFMUGenerator").filePath();
#ifdef _WIN32
	solverExecutable += ".exe";
#endif // _WIN32
	return solverExecutable;
}

QString SVSettings::view3dExecutable() {
	QString solverExecutable = QFileInfo(SVSettings::instance().m_installDir + "/View3D").filePath();
#ifdef _WIN32
	solverExecutable += ".exe";
#endif // _WIN32
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


void SVSettings::linuxDesktopIntegration(QWidget * parent,
							 const QString & iconLocation,
							 const QString & appname,               // SIM-VICUS
							 const QString & appIDname,             // simvicus
							 const QString & desktopAppComment,     // Building Energy Performance and District Simulation
							 const QString & desktopAppExec,        // /path/to/bin/SIM-VICUS
							 const QString & fileExtension          // vicus   (for *.vicus)
							 )
{
	// compose path to desktop-file, if existing, prompt user to "update" system integration, otherwise prompt to
	// "setup" integration

	// mimetype:     application/simvicus
	// icon-name:    simvicus         (prefix must match mimetype)
	// desktop-file: simvicus.desktop

	QString desktopFile = QDir::home().absoluteFilePath(QString(".local/share/applications/%1.desktop").arg(appIDname));
	if (QFileInfo::exists(desktopFile)) {
		int res = QMessageBox::question(parent, tr("Update Desktop Integration"), tr("Should the existing desktop integration and %2-file type association be updated?").arg(fileExtension),
										QMessageBox::Yes | QMessageBox::No);
		if (res == QMessageBox::No)
			return;
	}
	else {
		int res = QMessageBox::question(parent, tr("Update Desktop Integration"), tr("Should %1 set up the desktop integration and associate %2-file types with %1?").arg(appname).arg(fileExtension),
										QMessageBox::Yes | QMessageBox::No);
		if (res == QMessageBox::No)
			return;
	}


	// copy icon files, unless existing already
	QStringList iconSizes;
	iconSizes << "16" << "32" << "48" << "64" << "128" << "256" << "512";
	QString iconRootDir = QDir::home().absoluteFilePath(".local/share/icons/hicolor");
	foreach (QString s, iconSizes) {
		// path to source icon file
		QString iconFile = iconLocation + "/Icon_" + s + ".png";
		if (!QFile::exists(iconFile)) {
			qDebug() << QString("Missing icon file '%1'").arg(iconFile);
			continue;
		}

		// compose resolution-specific target path
		QString targetPath = QDir::home().absoluteFilePath(iconRootDir + "/%1x%1").arg(s);

		// copy desktop icon file, for example, 48x48 icon goes to 'hicolor/48x48/apps'
		QDir::home().mkpath( targetPath + "/apps"); // create path if not yet existing
		QString desktopIconTargetFile = targetPath + QString("/apps/%1.png").arg(appIDname);
		QFile::copy(iconFile, desktopIconTargetFile);

		// copy mimetype icon file, for example, 48x48 icon goes to 'hicolor/48x48/mimetypes' and is named 'application-<appIDname>'
		QDir::home().mkpath( targetPath + "/mimetypes"); // create path if not yet existing
		QString mimetypeIconTargetPath = targetPath + QString("/mimetypes/application-%1.png").arg(appIDname);
		QFile::copy(iconFile, mimetypeIconTargetPath);
	}

	// generate .desktop file, if it does not exist yet
	QString desktopFileContents =
			"[Desktop Entry]\n"
			"Name=%1\n"
			"Comment=%2\n"
			"Exec=%3\n"
			"Icon=%4\n"
			"Terminal=false\n"
			"Type=Application\n"
			"Categories=Science\n"
			"StartupNotify=true\n"
			"MimeType=application/%4\n";
	desktopFileContents = desktopFileContents.arg(appname, desktopAppComment, desktopAppExec, appIDname);
	QFile deskFile(desktopFile);
	deskFile.open(QFile::WriteOnly);
	QTextStream strm(&deskFile);
	strm << desktopFileContents;
	deskFile.setPermissions((QFile::Permission)0x7755);
	deskFile.close();

	// also create Mime file for file type associations for 'application/appIDname'
	QString mimeFileContents =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n"
			"	<mime-type type=\"application/%1\">\n"
			"		<comment>%2 project file</comment>\n"
			"		<glob pattern=\"*.%3\"/>\n"
			"	</mime-type>\n"
			"</mime-info>\n";
	mimeFileContents = mimeFileContents.arg(appIDname, appname, fileExtension);
	QString mimeDir = QDir::home().absoluteFilePath(".local/share/mime");
	QString mimeFile = mimeDir + QString("/packages/%1.xml").arg(appIDname);
	QFile mimeF(mimeFile);
	mimeF.open(QFile::WriteOnly);
	QTextStream strm2(&mimeF);
	strm2 << mimeFileContents;
	mimeF.close();

	// mime-type database update is still needed; if that doesn't work, we can't help it
	QProcess::execute("update-mime-database", QStringList() << mimeDir);

	QProcess::execute("update-icon-caches", QStringList() << iconRootDir);

	// Note: one still needs to logout/logon to make the icon association effective

	QMessageBox::information(parent, tr("Update Desktop Integration"), tr("Created application shortcut and registered file association. Changes will take effect after next login."));
}
