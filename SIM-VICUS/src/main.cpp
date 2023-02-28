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

#include "SVMainWindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QScreen>
#include <QPainter>
#include <QFontDatabase>

#if QT_VERSION >= 0x050A00
#include <QRandomGenerator>
#endif

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_ArgParser.h>
#include <IBK_BuildFlags.h>

#include <iostream>
#include <memory>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Directories.h>
#include <QtExt_AutoUpdater.h>

#include "SVMessageHandler.h"
#include "SVSettings.h"
#include "SVConstants.h"
#include "SVDebugApplication.h"
#include "SVStyle.h"

/*! qDebug() message handler function, redirects debug messages to IBK::IBK_Message(). */
void qDebugMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	(void) type;
	std::string contextstr;
	if (context.file != nullptr && context.function != nullptr)
		contextstr = "[" + std::string(context.file) + "::" + std::string(context.function) + "]";
	IBK::IBK_Message(msg.toStdString(), IBK::MSG_DEBUG, contextstr.c_str(), IBK::VL_ALL);
}


int main(int argc, char *argv[]) {
	const char * const FUNC_ID = "[main]";

	QtExt::Directories::appname = "SIM-VICUS";
	QtExt::Directories::devdir = "SIM-VICUS";

	// Compose program name using the always use the major.minor version variant,
	// since this string is used to identify the registry/config file location.
	const QString ProgramVersionName = QString("SIM-VICUS %1").arg(VICUS::VERSION);

	// *** Create and initialize setting object ***

	// create global settings object, from here until end-of-program-life accessible via SVSettings::instance()

	SVSettings settings(ORG_NAME, ProgramVersionName);
	settings.setDefaults();
	settings.read();

	// if we have just upgraded to a new version, try to import settings from the last minor version
	if (settings.m_versionIdentifier.isEmpty() && settings.m_lastProjectFile.isEmpty()) {
		unsigned int major, minor, patch;
		IBK::decode_version_number(VICUS::VERSION, major, minor, patch);
		for (int i=(int)minor-1; i>0; --i) {
			QString VersionName = QString("SIM-VICUS %1.%2").arg(major).arg(i);
			settings.m_appName = VersionName;
			settings.read();
			if (!settings.m_versionIdentifier.isEmpty() || !settings.m_lastProjectFile.isEmpty())
				break;
		}
		settings.m_appName = ProgramVersionName;
	}
	settings.m_versionIdentifier = VICUS::VERSION;

	// set QApplication configuration
	// IMPORTANT: this has to be done before QApplication (and thus SVDebugApplication) is instantiated
	if( settings.m_useHighDPIScaling ) {
		// We have to do this before our QApplication is initialized
#if QT_VERSION >= 0x050E00
		QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	}

	// create wrapped-QApplication class (to catch rogue exceptions)
	SVDebugApplication a(argc, argv);

	// install message handler to catch qDebug()
	qInstallMessageHandler(qDebugMsgHandler);

	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_UNIX)
	setlocale(LC_NUMERIC,"C");
#endif

	qApp->setWindowIcon(QIcon(":/logo/icons/Icon_64.png"));
	qApp->setApplicationName(ProgramVersionName);
	settings.m_ratio = qApp->devicePixelRatio();
	// note, that application path has to be reset, because QtExtSettings::setDefaults()
	// was called before SVDebugApplication was called and QApplication is initialized
	settings.m_installDir = qApp->applicationDirPath();

	// disable ? button in windows
#if QT_VERSION >= 0x050A00
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

	// initialize resources in dependent libraries
	Q_INIT_RESOURCE(QtExt);

	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(QtExt::Directories::userDataDir());

	// create global message handler object, from here until end-of-program-life accessible via IBK::MessageHandlerRegistry::instance()
	SVMessageHandler messageHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler( &messageHandler );
	std::string errmsg;
	messageHandler.openLogFile(QtExt::Directories::globalLogFile().toStdString(), false, errmsg);

	// adjust log file verbosity
	messageHandler.setConsoleVerbosityLevel( settings.m_userLogLevelConsole );
	messageHandler.setLogfileVerbosityLevel( settings.m_userLogLevelLogfile );

	// *** Read databases ***
	settings.m_db.readDatabases();

	// *** Style Init ***

	SVStyle style; // constructor sets up most of the initialization

	// *** Initialize Command Line Argument Parser ***
	IBK::ArgParser argParser;
	settings.updateArgParser(argParser);
	argParser.setAppName(ProgramVersionName.toStdString());
	argParser.m_appname = "SIM-VICUS";
	argParser.m_syntaxArguments = "[flags] [options] [<project file>]";
	// configure man page output
	argParser.m_manManualName = "SIM-VICUS Manual";
	argParser.m_manReleaseDate = NANDRAD::RELEASE_DATE;
	argParser.m_manVersionString = VICUS::LONG_VERSION;
	argParser.m_manShortDescription = "Building energy and district simulation modeling environment";
	// Note: mind the line breaks that end format commands!
	argParser.m_manLongDescription = ".B SIM-VICUS\n"
			"is a graphical modeling environment for building energy models and district networks.";

	// *** Apply command line arguments ***
	argParser.parse(argc, argv);
	// handle default arguments (--help)
	if (argParser.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;

	settings.applyCommandLineArgs(argParser);


	// *** Check for auto-update files ***
#if defined(Q_OS_WIN)
	QtExt::AutoUpdater autoUpdater;
	if (autoUpdater.installUpdateWhenAvailable(QtExt::Directories::updateFilePath()))
		return EXIT_SUCCESS;
#endif

	// *** Install translator ***
	QtExt::LanguageHandler::instance().setup(SVSettings::instance().m_organization,
											 SVSettings::instance().m_appName,
											 "SIM-VICUS" );
	if (argParser.hasOption("lang")) {
		std::string dummy = argParser.option("lang");
		QString langid = QString::fromStdString(dummy);
		if (langid != QtExt::LanguageHandler::instance().langId()) {
			IBK::IBK_Message( IBK::FormatString("Installing translator for language: '%1'.\n")
								.arg(langid.toStdString()),
								IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			QtExt::LanguageHandler::instance().installTranslator(langid);
		}
	}
	else {
		QtExt::LanguageHandler::instance().installTranslator(QtExt::LanguageHandler::langId());
	}

	// set default language in IBK MultiLanguageString
	IBK::MultiLanguageString::m_language = QtExt::LanguageHandler::langId().toStdString();


	// *** Create and show splash-screen ***
	std::unique_ptr<QSplashScreen> splash;

	if (!settings.m_flags[SVSettings::NoSplashScreen]) {
		QPixmap pixmap;

	int imageCount = 6;

#if QT_VERSION >= 0x050A00
		int pixmapIdx = QRandomGenerator::global()->bounded(0,imageCount);
#else

		std::srand(std::time(nullptr));
		int pixmapIdx = std::rand()*imageCount/RAND_MAX;
#endif

		pixmap.load(QString(":/gfx/splashscreen/SIMVICUS-Logo-Startscreen-%1.png").arg(pixmapIdx));

		// is needed for high dpi screens to prevent bluring
		double ratio = a.primaryScreen()->devicePixelRatio();
		pixmap.setDevicePixelRatio(ratio);

		// Load custom font
		int id = QFontDatabase::addApplicationFont(":/fonts/Manrope-Bold.otf");
		QString family = QFontDatabase::applicationFontFamilies(id).at(0);
		QFont font(family);
		font.setBold(true);
		font.setPixelSize(16/ratio);

		// Create painter
		QPainter painter(&pixmap);
		painter.setFont(font);
		painter.setRenderHint(QPainter::Antialiasing);

		// draw "Version"
		painter.setPen(Qt::white);
		painter.drawText(615.0/ratio, 548.0/ratio, 180.0/ratio, 30.0/ratio, Qt::AlignRight, QString("VERSION %1").arg(VICUS::VERSION));

		id = QFontDatabase::addApplicationFont(":/fonts/Manrope-Medium.otf");
		family = QFontDatabase::applicationFontFamilies(id).at(0);
		font = QFont(family);
		font.setPixelSize(14/ratio);
		painter.setFont(font);

		// Add beta information
		painter.setPen(QColor("#CE4A4A"));
		if (VICUS::VERSION[0] == '0')
			painter.drawText(615.0/ratio, 572.0/ratio, 180.0/ratio, 30.0/ratio, Qt::AlignRight, QString("BETA-VERSION"));
		painter.end();

		// show splash screen
		splash.reset(new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::SplashScreen));
		splash->show();
		QTimer::singleShot(5000, splash.get(), SLOT(close()));
	}


	// *** Setup and show MainWindow and start event loop ***
	int res;
	try { // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		SVMainWindow w;
		a.m_mainWindow = &w; // set main window pointer for event filtering

		// start event loop
		res = a.exec();
	} // here our mainwindow dies, main window goes out of scope and UI goes down -> destructor does ui and thread cleanup
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}
	a.m_mainWindow = nullptr; // no more main window

	// return exit code to environment
	return res;
}
