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

#include "SVPostProcHandler.h"

#include <QProcess>

#include <IBK_FormatString.h>

#include "SVSettings.h"

#ifdef _WIN32

struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};


BOOL is_main_window(HWND handle) {
	return (GetWindow(handle, GW_OWNER) == (HWND)nullptr) && IsWindowVisible(handle);
}


BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam) {
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}


HWND find_main_window(unsigned long process_id) {
	handle_data data;
	data.process_id = process_id;
	data.window_handle = nullptr;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

#else
	#include <sys/types.h>
	#include <signal.h>
#endif // _WIN32


SVPostProcHandler::SVPostProcHandler() :
#ifdef _WIN32
	m_postProcHandle(nullptr)
#else
	m_postProcHandle(0)
#endif // _WIN32
{

}


int SVPostProcHandler::reopenIfActive() {
#if _WIN32
	if (m_postProcHandle != nullptr) {
		DWORD exitCode;
		BOOL res = GetExitCodeProcess(m_postProcHandle, &exitCode);
		if (!res) {
			m_postProcHandle = nullptr;
			return 1;
		}
		if (exitCode == STILL_ACTIVE) {
			DWORD postProcId = GetProcessId(m_postProcHandle);
			HWND postProcWinHandle = find_main_window(postProcId);
			BringWindowToTop(postProcWinHandle);
			return 0;
		}
	}
#else
	// send 0 kill signal and see if the error code tells is, that the process is still there
	if (m_postProcHandle != 0) {
#if defined(Q_OS_MAC)
//		NSRunningApplication * pp_app = runningApplicationWithProcessIdentifier(m_postProcHandle);
#else
		int killres = kill(m_postProcHandle, 0);
		if (killres == 0) {
			// process lives!
			// TODO : raise window to front
			return 0;
		}
		else {
			m_postProcHandle = 0;
			return 1;
		}
#endif
	}
#endif
	return 1;
}


bool SVPostProcHandler::spawnPostProc(const std::string & sessionFile) {
	FUNCID(SVPostProcHandler::spawnPostProc); (void)FUNC_ID;

#if _WIN32
	IBK::FormatString cmdLine;
	if (sessionFile.empty()) {
		cmdLine = IBK::FormatString("\"%1\"")
				.arg(SVSettings::instance().m_postProcExecutable.toStdString());
	}
	else {
		cmdLine = IBK::FormatString("\"%1\" \"%2\"")
		.arg(SVSettings::instance().m_postProcExecutable.toStdString())
		.arg(sessionFile);
	}


	// Use WinAPI to create a PostProc process
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;

	std::string cmd = cmdLine.str();
	// Mind: cmdLine is still utf8 encoded
	std::wstring wcmd = IBK::UTF8ToWstring(cmd);

	// Start the child process.
	if( !CreateProcessW( nullptr,	// No module name (use command line).
		&wcmd[0],					// Command line.
		nullptr,					// Process handle not inheritable.
		nullptr,					// Thread handle not inheritable.
		FALSE,						// Set handle inheritance to FALSE.
		lower_priority,				// Create with priority lower then normal.
		nullptr,					// Use parent's environment block.
		nullptr,					// Use parent's starting directory.
		&si,						// Pointer to STARTUPINFO structure.
		&pi )						// Pointer to PROCESS_INFORMATION structure.
	)
	{
		return false;
	}
	// store process handle for later
	m_postProcHandle = pi.hProcess;
	return true;
#else
	// spawn post-proc process
	QProcess p;
	p.setProgram(SVSettings::instance().m_postProcExecutable);
	QStringList args;
	if (!sessionFile.empty())
		args += QString::fromStdString(sessionFile);
	bool res = p.startDetached(SVSettings::instance().m_postProcExecutable, args, QString(), &m_postProcHandle);
	if (!res)
		IBK::IBK_Message(IBK::FormatString("Post-processing '%1' could not be started.")
						 .arg(SVSettings::instance().m_postProcExecutable.toStdString()), IBK::MSG_WARNING, FUNC_ID);
	return res;
#endif
}
