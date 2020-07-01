// IBK Library - Collection of classes, functions and algorithms
//               for development of numeric simulations
//
// Copyright (C) 2007 Andreas Nicolai, Andreas.Nicolai@gmx.net
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

#include "IBK_configuration.h"

#include "IBK_dllhandler.h"
#include "IBK_StringUtils.h"

namespace IBK {

DllHandler::~DllHandler() {
	if (m_hdll != NULL)
		FreeLibrary(m_hdll);
}

const char* DllHandler::getVersion() {
	// if not done yet, load Dll into memory
	if (m_hdll == NULL && !loadDll())
		return "";

	return m_getVersion();
}

/// Retrieves version numbers.
/// Returns -1 for all revision numbers if Dll could not be loaded.
void DllHandler::getVersionNumbers(int & major, int & minor, int & revision) {
	major = minor = revision = -1;
	std::string v = getVersion();
	if (v.empty())
		return;

	size_t pos = v.find('.');
	try {
		if (pos == std::string::npos)
			throw std::runtime_error("Major version number missing.");

		major = string2val<int>(v.substr(0, pos));
		size_t pos2 = v.find('.', pos+1);
		if (pos2 == std::string::npos)
			throw std::runtime_error("Minor version number missing.");
			
		minor = string2val<int>(v.substr(pos+1, pos2-pos));
		revision = string2val<int>(v.substr(pos2+1));
	}
	catch (...) {
		// do nothing
	}
}

bool DllHandler::loadDll() {
	// load dll into memory
	if( m_hdll != 0)
		m_hdll = LoadLibraryA(m_dllPath.c_str());
	if (m_hdll == 0)
		return false;

	// retrieve function pointers to functions declared in dll
	m_getVersion = reinterpret_cast<Fget_version*>(GetProcAddress(m_hdll, "_get_version"));
	if (m_getVersion == NULL)
		return false;

	return true;
}

} // namespace IBK
