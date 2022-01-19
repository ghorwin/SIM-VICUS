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

#ifndef SVDebugApplicationH
#define SVDebugApplicationH

#include <QApplication>

class SVMainWindow;

/*! This class catches all exceptions thrown during eventloop execution.
	It basically programmed for debug purposes.
*/
class SVDebugApplication : public QApplication {
	Q_OBJECT
public:
	/*! ctor relay. */
	SVDebugApplication( int & argc, char ** argv ) :
		QApplication( argc, argv )
	{
	}

	/*! We just reimplement QApplication::notify() to catch all exceptions and allow setting a breakpoint here. */
	bool notify( QObject *recv, QEvent *e );

	/*! Set to true in case of a critical exception */
	bool m_aboutToTerminate = false;

	/*! Pointer to the main window, needed to relay global key presses to window, when the scene has focus. */
	SVMainWindow * m_mainWindow = nullptr;
};

#endif // SVDebugApplicationH
