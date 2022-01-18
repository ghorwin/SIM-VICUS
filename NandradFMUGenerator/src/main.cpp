#include <QApplication>

#include <QFileInfo>
#include <QIcon>
#include <QDir>

#include <IBK_Exception.h>
#include <IBK_ArgParser.h>
#include <IBK_messages.h>

#include <iostream>

#include "NandradFMUGeneratorWidget.h"

int main(int argc, char *argv[]) {
	FUNCID(main);

	// create QApplication
	QApplication a(argc, argv);

	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_UNIX)
	setlocale(LC_NUMERIC,"C");
#endif

	// disable ? button in windows
#if QT_VERSION >= 0x050A00
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#elif QT_VERSION >= 0x050600
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	IBK::ArgParser args;
	args.m_appname = "NandradFMUGenerator";
	args.addOption(0, "generate", "Generates this FMU from the provided project file (requires project file argument).", "FMU-model-name", "");
	args.parse(argc, argv);
	if (args.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;

	// *** Setup and show main widget and start event loop ***
	int res;
	try {

		NandradFMUGeneratorWidget w;

		// For now we assume install dir to be same as NandradSolver dir
		w.m_installDir					= QFileInfo(argv[0]).dir().absolutePath();
		w.m_nandradSolverExecutable		= QFileInfo(argv[0]).dir().absoluteFilePath("NandradSolver");
		IBK::IBK_Message(IBK::FormatString("Using NandradSolver: '%1'\n").arg(w.m_nandradSolverExecutable.toStdString()),
						 IBK::MSG_PROGRESS, FUNC_ID);

		// if started as: NandradFMUGenerator /path/to/projectFile.nandrad
		// we copy /path/to/projectFile.nandrad as path to NANDRAD
		if (args.args().size() > 1) {
			std::string projectFile = args.args()[1];
			if (projectFile.find("file://") == 0)
				projectFile = projectFile.substr(7);
			w.m_nandradFilePath = IBK::Path(projectFile);
		}

		// check for: NandradFMUGenerator --generate=MyModel  /path/to/MyModelProject.nandrad
		if (args.hasOption("generate")) {
			if (!w.m_nandradFilePath.isValid()) {
				throw IBK::Exception("Project file argument expected when using '--generate'.", FUNC_ID);
			}
			w.m_silent = true; // set widget into silent mode - this also indicates that we are in "scripted" mode
			w.m_autoExportModelName = QString::fromStdString(args.option("generate"));
		}

		// handle initial state (reading project given by command line etc.)
		w.resize(1600,800);
		w.show(); // show widget
		w.init();

		// in scripted mode, we are already done, no need to start an event loop
		if (w.m_silent)
			return EXIT_SUCCESS;

		// start event loop
		res = a.exec();

	} // here our widget dies, main window goes out of scope and UI goes down -> destructor does ui and thread cleanup
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}

	// return exit code to environment
	return res;
}
