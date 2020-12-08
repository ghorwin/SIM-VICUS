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

#ifndef IBK_dllhandlerH
#define IBK_dllhandlerH

#include <windows.h>
#include <string>

namespace IBK {

/// Base class for Dll handler classes.
/// This class implements common functionality, 
class DllHandler {
	typedef const char  * Fget_version();
public:
	DllHandler(const std::string& dllpath) : m_dllPath(dllpath), m_hdll(NULL) {}
	virtual ~DllHandler();

	/// Retrieves the version number.
	/// Returns an empty string if Dll could not be loaded. This can
	/// be used to test for existing Dlls. Use getVersionNumbers() to
	/// get numeric representations of version numbers.
	/// @see  getVersionNumbers()
	virtual const char* getVersion();

	/// Retrieves version numbers.
	/// Returns -1 for all revision numbers if Dll could not be loaded.
	virtual void getVersionNumbers(int & major, int & minor, int & revision);
	
protected:

	/// Implements basic Dll loading functionality.
	/// Call this function first when re-implementing this functions.
	virtual bool loadDll();

	Fget_version				*m_getVersion;				///< Function pointer to get_version() function in Dll
	std::string 				m_dllPath;					///< Path to dll file
	HINSTANCE 					m_hdll;						///< Handle to loaded Dll
};

} // namespace IBK

#endif // IBK_dllhandlerH

