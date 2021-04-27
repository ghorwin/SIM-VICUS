#include <QApplication>

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


	// *** Setup and show MainWindow and start event loop ***
	int res;
	try { // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		NandradFMUGeneratorWidget w;
		w.show();

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
