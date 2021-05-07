#include <QApplication>

#include <QFileInfo>
#include <QDir>

#include <IBK_Exception.h>

#include "NandradFMUGeneratorWidget.h"

int main(int argc, char *argv[]) {

	// create QApplication
	QApplication a(argc, argv);

	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_UNIX)
	setlocale(LC_NUMERIC,"C");
#endif

	qApp->setApplicationName("NANDRAD FMU Generator");

	// disable ? button in windows
#if QT_VERSION >= 0x050A00
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#elif QT_VERSION >= 0x050600
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif


	// *** Setup and show main widget and start event loop ***
	int res;
	try {

		NandradFMUGeneratorWidget w;

		// TODO : For now we assume install dir to be same as NandradSolver dir
		w.m_installDir					= QFileInfo(argv[0]).dir().absolutePath();
		w.m_nandradSolverExecutable		= QFileInfo(argv[0]).dir().absoluteFilePath("NandradSolver");

		// if started as: NandradFMUGenerator /path/to/projectFile.nandrad
		// we copy /path/to/projectFile.nandrad as path to NANDRAD
		if (argc > 1) {
			w.m_nandradFilePath = IBK::Path(argv[1]);
		}

		w.init();
		w.show(); // show widget

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
