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

#ifndef SVPostProcHandlerH
#define SVPostProcHandlerH

#ifdef _WIN32
	#undef UNICODE
	#include <windows.h>
#endif

#include <string>
#include <QProcess>

/*! Wrapper around window-specific process handling and Linux/Mac functionality.
*/
class SVPostProcHandler {
public:
	SVPostProcHandler();

	/*! Tests, if a post proc process is already active.
		If a process is active, it will bring the postproc main window to top.
		If the process is not active, it will return 1;
		In case of any WINAPI error (or *nix problem), the function returns 2.
		The function returns 0 if the postproc window could be shown correctly.
	*/
	int reopenIfActive();

	/*! Spawns a new instance of the postprocessing and passes
		the session file if not empty (only for PostProc2).
		\return Returns true, if the process could be created successfully.
	*/
	bool spawnPostProc(const std::string & sessionFile);

#ifdef _WIN32
	HANDLE m_postProcHandle;
#else
	qint64 m_postProcHandle; // process ID on linux/mac
#endif
};

#endif // SVPostProcHandlerH
