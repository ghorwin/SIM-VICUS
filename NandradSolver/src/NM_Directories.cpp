/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#include "NM_Directories.h"

#ifdef _WIN32
  #include <windows.h>
  #include <shlobj.h>
#endif // _WIN32

#include <functional>
#include <cstdlib>
#include <ctime>

#include <IBK_assert.h>
#include <IBK_FormatString.h>
#include <IBK_FileUtils.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>


namespace NANDRAD_MODEL {

Directories::Directories() {
	// create all global pathes
	#ifdef _WIN32
		// retreive windows folder for temporary data 
		wchar_t tmpDirWChar[MAX_PATH];

		if (GetTempPathW(MAX_PATH, tmpDirWChar)) {
			char tmpDirChar[MAX_PATH];
			wcstombs(tmpDirChar, tmpDirWChar, sizeof(tmpDirChar));
			// on Windows, we store temporary data at %TMP%
			m_tmpDir = IBK::Path(tmpDirChar);
		}

		// retreive windows folder for local user data 
		wchar_t usrDirWChar[ MAX_PATH ];

		if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, usrDirWChar) == S_OK) {
			char usrDirChar[MAX_PATH];
			wcstombs(usrDirChar, usrDirWChar, sizeof(usrDirChar));
			// on Windows, we store user data at %HOME%/AppData/Local
			m_userDataDir = IBK::Path(usrDirChar + std::string("/../IBK/Nandrad"));

			// create directory if not existent
			if(!m_userDataDir.isDirectory()) {
				IBK::Path::makePath(m_userDataDir);
			}
		}

	#else
		// on Unix/Mac OS we use global tmp dir
		m_tmpDir = IBK::Path("/tmp");

		// on Unix/Mac OS we store user data under home directory
		 const char *homeDir = getenv("HOME");

		if(homeDir) {
			m_userDataDir = IBK::Path(homeDir + std::string("/.ibk/Nandrad"));
			// create directory if not existent
			if(!m_userDataDir.isDirectory()) {
				IBK::Path::makePath(m_userDataDir);
			}
		}

	#endif // _WIN32
}


void Directories::create(const IBK::Path & projectRootPath) {

	const char * const FUNC_ID = "[Directories::create]";

	m_rootDir.clear();
	m_logDir.clear();
	m_varDir.clear();
	m_resultsDir.clear();
	m_slavesDir.clear();

	// create solver base directory
	if ( !projectRootPath.exists() ) {
		if ( !IBK::Path::makePath( projectRootPath) ) {
			throw IBK::Exception( IBK::FormatString("Cannot create directory '%1'. Perhaps missing priviliges?").arg(projectRootPath), FUNC_ID);
		}
	}

	IBK::Path logPath = projectRootPath / "log";
	if ( ! logPath.exists() ) {
		if ( !IBK::Path::makePath(logPath)) {
			throw IBK::Exception("Cannot create directory '" + logPath.str() + "'. Perhaps missing priviliges?", FUNC_ID);
		}
	}

	IBK::Path varPath = projectRootPath / "var";
	if ( !varPath.exists() ) {
		if ( !IBK::Path::makePath(varPath)) {
			throw IBK::Exception("Cannot create directory '" + varPath.str() + "'. Perhaps missing priviliges?", FUNC_ID);
		}
	}

	IBK::Path outPath = projectRootPath / "results";
	if ( !outPath.exists() ) {
		if ( !IBK::Path::makePath(outPath)) {
			throw IBK::Exception("Cannot create directory '" + outPath.str() + "'. Perhaps missing priviliges?", FUNC_ID);
		}
	}


	m_rootDir = projectRootPath;
	m_logDir = logPath;
	m_varDir = varPath;
	m_resultsDir = outPath;
	m_heatingDesignRootDir = IBK::Path(projectRootPath.str()
		+ std::string("_heatingDesignDay") );
	m_coolingDesignRootDir = IBK::Path(m_rootDir.str() 
		+ std::string("_coolingDesignDay") );
	m_heatingDesignResultsDir = m_heatingDesignRootDir / "results";
	m_coolingDesignResultsDir = m_coolingDesignRootDir / "results";

	// create directory for temporary files
}

void Directories::createFMUSlavesDirectory() {
	const char * const FUNC_ID = "[Directories::createFMUSlavesDirectory]";

	m_slavesDir.clear();

	IBK_ASSERT(m_rootDir.exists());

	IBK::Path slavePath = m_rootDir / "slaves";
	if (!slavePath.exists()) {
		if (!IBK::Path::makePath(slavePath)) {
			throw IBK::Exception("Cannot create directory '" + slavePath.str() + "'. Perhaps missing priviliges?", FUNC_ID);
		}
	}

	m_slavesDir = slavePath;
}


} // namespace NANDRAD_MODEL

