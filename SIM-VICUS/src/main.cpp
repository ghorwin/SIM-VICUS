#include "DelMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QSplashScreen>
#include <QTimer>
#include <QLocale>
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_ArgParser.h>
#include <IBK_BuildFlags.h>

#include <memory>
#include <iostream>

#include <QtExt_LanguageHandler.h>
#include <QtExt_AutoUpdater.h>
#include <QtExt_Directories.h>

#include <DELPHIN_Constants.h>

#include "DelMessageHandler.h"
#include "DelSettings.h"
#include "DelConstants.h"
#include "DelDebugApplication.h"
#include "DelConversion.h"

#if QT_VERSION >= 0x050000
void qDebugMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	(void) type;
	(void) context;
	std::cout << msg.toStdString() << std::endl;
}
#else
void qDebugMsgHandler(QtMsgType type, const char *msg) {
	(void) type;
	std::cout << msg << std::endl;
}
#endif


int main(int argc, char *argv[]) {
	const char * const FUNC_ID = "[main]";

	QtExt::Directories::appname = "Delphin6";
	QtExt::Directories::devdir = "Delphin6";

	DelDebugApplication a(argc, argv);

#if QT_VERSION >= 0x050000
	qInstallMessageHandler(qDebugMsgHandler);
#else
	qInstallMsgHandler(qDebugMsgHandler);
#endif



	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_UNIX)
	setlocale(LC_NUMERIC,"C");
#endif

	qApp->setWindowIcon(QIcon(":/gfx/delphin_icon_black_48x48.png"));
	qApp->setApplicationName(PROGRAM_NAME);

	// disable ? button in windows
#if QT_VERSION >= 0x050A00
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	// initialize resources in dependent libraries
	Q_INIT_RESOURCE(QtExt);
	Q_INIT_RESOURCE(SciChart);


	// *** Create and initialize setting object of DSix Application ***
	DelSettings settings(ORG_NAME, PROGRAM_NAME);
	settings.setDefaults();
	settings.read();

	// add ${Install Directory} placeholder
	QFileInfo prjFileInfo(argv[0]);
	settings.m_defaultPathPlaceholders[DelSettings::DB_InstallDir] = prjFileInfo.dir().absolutePath();

	// customize application font
	unsigned int ps = DelSettings::instance().m_fontPointSize;
	if (ps != 0) {
		QFont f(qApp->font());
		f.setPointSize((int)ps);
		qApp->setFont(f);
	}

	// *** Initialize Command Line Argument Parser ***
	IBK::ArgParser argParser;
	settings.updateArgParser(argParser);
	argParser.setAppName(PROGRAM_NAME);

	// *** Apply command line arguments ***
	argParser.parse(argc, argv);
	// handle default arguments (--help)
	if (argParser.flagEnabled("help")) {
		argParser.printHelp(std::cout);
		return EXIT_SUCCESS;
	}
	settings.applyCommandLineArgs(argParser);


	// *** Check for auto-update files ***
#if defined(Q_OS_WIN)
	QtExt::AutoUpdater autoUpdater;
	if (autoUpdater.installUpdateWhenAvailable(QtExt::Directories::updateFilePath()))
		return EXIT_SUCCESS;
#endif

	// *** Create desktop icon
#ifdef Q_OS_LINUX
	QString desktopFileContents =
			"[Desktop Entry]\n"
			"Name=DELPHIN %1\n"
			"Comment=Hygrothermal simulation program\n"
			"Exec=%2/Delphin6\n"
#ifdef IBK_DEPLOYMENT
			"Icon=%3/Delphin6_64.png\n"
#else
			"Icon=%3/gfx/delphin_icon_black_64x64.png\n"
#endif
			"Terminal=false\n"
			"Type=Application\n"
			"Categories=Science;Engineering;Physics\n"
			"StartupNotify=true\n";
	desktopFileContents = desktopFileContents.arg(DELPHIN::LONG_VERSION).arg(settings.m_installDir)
			.arg(QtExt::Directories::resourcesRootDir());
	QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	if (!dirs.empty()) {
		IBK::IBK_Message("Creating 'delphin6.desktop' file.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		QString desktopFile = dirs[0] + "/delphin6.desktop";
		if (!QFile(desktopFile).exists()) {
			QFile deskFile(desktopFile);
			deskFile.open(QFile::WriteOnly);
			QTextStream strm(&deskFile);
			strm << desktopFileContents;
			deskFile.setPermissions((QFile::Permission)0x755);
			deskFile.close();
		}
		// also create Mime file for file type associations
		QString mimeFileContents =
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n"
				"	<mime-type type=\"application/x-delphin6\">\n"
				"		<comment>DELPHIN 6 project file</comment>\n"
				"		<generic-icon name=\"delphin6\"/>\n"
				"		<glob pattern=\"*.d6p\"/>\n"
				"		<glob pattern=\"*.d6pp\"/>\n"
				"	</mime-type>\n"
				"</mime-info>\n";
		QString mimeFile = dirs[0] + "/../mime/packages/x-delphin6.xml";
		if (!QFile(mimeFile).exists()) {
			QFile mimeF(mimeFile);
			mimeF.open(QFile::WriteOnly);
			QTextStream strm(&mimeF);
			strm << mimeFileContents;
			mimeF.close();
		}
	}

#endif


	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(QtExt::Directories::userDataDir());

	DelMessageHandler messageHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler( &messageHandler );
	std::string errmsg;
	messageHandler.openLogFile(QtExt::Directories::globalLogFile().toUtf8().data(), false, errmsg);
	messageHandler.setConsoleVerbosityLevel( settings.m_userLogLevelConsole );
	messageHandler.setLogfileVerbosityLevel( settings.m_userLogLevelLogfile );


	// *** Install translator ***
	QtExt::LanguageHandler::instance().setup(DelSettings::instance().m_organization,
											 DelSettings::instance().m_appName,
											 QtExt::Directories::translationsDir(),
											 "Delphin6" );
	if (argParser.hasOption("lang")) {
		std::string dummy = argParser.option("lang");
		QString langid = utf82QString(dummy);
		if (langid != QtExt::LanguageHandler::instance().langId()) {
			IBK::IBK_Message( IBK::FormatString("Installing translator for language: '%1'.\n")
								.arg(langid.toUtf8().data()),
								IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			QtExt::LanguageHandler::instance().installTranslator(langid);
		}
	}
	else {
		QtExt::LanguageHandler::instance().installTranslator(QtExt::LanguageHandler::langId());
	}


	// *** Create and show splash-screen ***
#if __cplusplus <= 199711L
	std::auto_ptr<QSplashScreen> splash;
#else // __cplusplus <= 199711L
	std::unique_ptr<QSplashScreen> splash;
#endif // __cplusplus <= 199711L
	if (!settings.m_flags[DelSettings::NoSplashScreen]) {
		QPixmap pixmap;
		pixmap.load(":/gfx/splashscreen/Delphin_SplashScreen.png","PNG");
		splash.reset(new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::SplashScreen));
		splash->show();
		QTimer::singleShot(5000, splash.get(), SLOT(close()));
	}

#if 0
	// "end of service life" code
	if (DelSettings::instance().m_versionExpired) {
		QMessageBox::information(NULL, qApp->translate("main", "Update available"),
								 qApp->translate("main", "A newer version for this software is available. Please visit "
									"bauklimatik-dresden.de to download the update!"));
		return EXIT_FAILURE;
	}
#endif


	// *** Setup and show MainWindow and start event loop ***
	int res;
	try { // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		DelMainWindow w;

		// add user settings related window resize at program start
#if defined(Q_OS_WIN)
		QTimer::singleShot(10, &w, SLOT(showMaximized()));
#elif defined(Q_OS_LINUX)
		QTimer::singleShot(10, &w, SLOT(show()));
#else
		QTimer::singleShot(10, &w, SLOT(show()));
#endif

		// start event loop
		res = a.exec();
	} // here our mainwindow dies, main window goes out of scope and UI goes down -> destructor does ui and thread cleanup
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}

	// return exit code to environment
	return res;
}
