/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "Path.h"

#if defined(_WIN32)
	#include <Windows.h>
	#include <direct.h>
	#include <sys/types.h>
	#include <cmath>
#else
	#include <unistd.h>
	#include <utime.h>
	#include <errno.h>
	#include <dirent.h>
#endif

#include <sys/stat.h>

#include <assert.h>
#include <locale>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <string>
#include <iostream>


//#include "utf8/utf8.h"


Path::Path()
{}


Path::Path(const std::string& path) {
	set(path);
}


Path& Path::operator=(const std::string& path) {
	set(path);
	return *this;
}


Path& Path::operator=(const char * const path) {
	set(std::string(path));
	return *this;
}


void Path::clear() {
	m_path.clear();
}


std::string Path::str() const {
	return m_path;
}

std::string Path::osStr() const {
	std::string result = m_path;
#if defined(_WIN32)
	std::replace(result.begin(), result.end(), '/', '\\');
#else
// do nothing
#endif
	return result;
}


#if defined(_WIN32)
std::wstring Path::wstr() const {
	return UTF8ToWstring(m_path);
}

std::wstring Path::wstrOS() const {
	std::string result = m_path;
	std::replace(result.begin(), result.end(), '/', '\\');
	return UTF8ToWstring(result);
}

#endif



Path Path::absolutePath() const {

	// Removes relative parts if possible.
	//	A leading . or .. will be replaced by using current directory.

	// no path exists
	if( m_path.empty())
		return *this;

	Path result(m_path);

	result.removeRelativeParts();

	// the path is already absolute
	if( isAbsolute())
		return result;

	Path currentPath = current();
	// in case of './path' remove the './'
	if( result.m_path.size() > 1 && result.m_path[0] == '.' && result.m_path[1] == '/')
		result.m_path = result.m_path.substr(2);
	currentPath = currentPath / result.m_path;

	result = currentPath;
	result.removeRelativeParts();

	return result;
}

bool Path::canCreateRelativePath(const Path& toPath, std::string& errstr) const {
	if (!isValid() ) {
		errstr = "Empty/invalid path.";
		return false;
	}

	if( !toPath.isValid() ) {
		errstr = "Other path argument is an empty/invalid path.";
		return false;
	}

	Path absolute = absolutePath();
	Path absoluteToPath = toPath.absolutePath();

	if ( !absolute.isValid() || !absoluteToPath.isValid() ) {
		errstr = "Cannot create absolute path.";
		return false;
	}

	// pathes are equal, no relative path possible
	if (absolute == absoluteToPath)
		return true;

	std::string driveCurrent = absolute.drive();
	std::string driveRelPath = absoluteToPath.drive();

	// cannot create relative path for windows if drives are different
	if (driveCurrent != driveRelPath) {
		errstr = "Different drives.";
		return false;
	}

	// isLinux is set when the path begins with a leading slash
	/// \todo check for network paths on windows if entered with slashes

	return true;
}


Path Path::relativePath(const Path& toPath) const {

	std::string errstr;
	if( !canCreateRelativePath(toPath, errstr))
		throw std::runtime_error(errstr);

	Path absolute = absolutePath();
	Path absoluteToPath = toPath.absolutePath();

	// pathes are equal, no relative path possible
	if (absolute == absoluteToPath)
		return *this;

	bool isLinux = false;
	std::vector<std::string> orgBranches;
	explode(absolute.m_path, orgBranches, "/");
	if (absolute.m_path[0] == '/') {
		isLinux = true;
		orgBranches.insert( orgBranches.begin(), "/" );
	}

	std::vector<std::string> toBranches;
	explode(absoluteToPath.m_path, toBranches, "/");
	if (absoluteToPath.m_path[0] == '/') {
		toBranches.insert( toBranches.begin(), "/" );
	}

	if (orgBranches.empty()) {
		throw std::runtime_error("Invalid original path: '" + m_path + "'");
	}

	if (toBranches.empty()) {
		throw std::runtime_error("Invalid to path: '" + m_path + "'");
	}

	unsigned int equalCount = 0;
	unsigned int minCount = (unsigned int)std::min<size_t>(orgBranches.size(), toBranches.size());
	for (unsigned int i=0; i<minCount; ++i, ++equalCount) {
		if (orgBranches[i] != toBranches[i])
			break;
	}

	try {

		Path finalPath;
		int backCount = (int)toBranches.size() - (int)equalCount;
		assert(backCount >= 0); // should not possible to get negative backCount

		if (backCount == 0) {
			if (isLinux)
				finalPath = Path("./"); // we must not have an empty final path, at the end we remove the leading ./ again
		}
		else {
			for (int i=0; i<backCount; ++i) {
				finalPath /= Path("..");
			}
		}

		int notEqualCount = (int)orgBranches.size() - (int)equalCount;
		assert(notEqualCount >= 0); // should not possible

		if (notEqualCount > 0)
			finalPath /= absolute.subBranch(equalCount, (unsigned int) notEqualCount); // add rest
		// if resulting path becomes an absolute path (on Linux/Mac), remove leading slash
		if (!finalPath.m_path.empty() && finalPath.m_path[0] == '/')
			finalPath.m_path.erase(0,1);
		return finalPath;
	}
	catch (std::runtime_error &ex) {
		throw std::runtime_error("Error reading schedule from file '" + toPath.str() + "': " + ex.what() );
	}
}


std::string Path::drive() const {

	if( m_path.size() < 2 || m_path[1] != ':')
		return "";

	std::string newDrive = m_path.substr(0,1) + ":";
	return newDrive;

}


Path Path::parentPath() const {

	std::string::size_type pos;
	std::string resPath = m_path;

	if (!isValid())
		throw std::runtime_error("Invalid path: '" + m_path + "'.");

	if (resPath.size() == 2) {
		pos = resPath.find(':');
		if (pos != std::string::npos) {
			throw std::runtime_error("No parent possible.");
		}
	}

	pos = resPath.rfind('/');
	if (pos != std::string::npos) {
		resPath = resPath.substr(0, pos);
		return Path(resPath);
	}
	else {
		return Path(".");
	}
}


bool Path::isValid() const {
	return !m_path.empty();
}

bool Path::exists() const {
#if defined(_WIN32)
	if(isDrive())
		return true;
#endif
	return directoryExists(m_path) != NoSuchDirectory;
}


bool Path::isAbsolute() const {
	return (m_path.size() >= 1 && m_path[0] == '/') || (m_path.size() >= 2 && m_path[1] == ':');
}


Path Path::filename() const {

	// IBK::Path("").filename() --> exception
	if (m_path.empty())
		throw std::runtime_error("Invalid path: '" + m_path + "'.");

	if (m_path.size() == 1){

		// IBK::Path(".")
		if (m_path == "." || m_path == "/") {
			return Path("");
		}

	} else if ( (m_path.size() == 2) ){

		// IBK::Path("C:").filename() --> ""  (weil "C:" == "C:\\" == "C:/" == "C:" + "/" + "")
		// IBK::Path("./")
		if ( (m_path[1] == ':') || (m_path == "./") ){
			return Path("");
		}

	} else {

		// IBK::Path("C:\\blubb\\bla\\blub").filename() --> "blub" <- constructor handles this
		// IBK::Path("./blub.txt").filename() --> "blub.txt"
		// IBK::Path("../blub.txt").filename() --> "blub.txt"
		// search for last / in string and create a substring
		size_t pos = m_path.rfind('/');
		if (pos != std::string::npos){
			return Path( m_path.substr( pos+1, std::string::npos ) );
		}

	}

	// IBK::Path("blub.txt").filename() --> "blub.txt"
	return *this;

}


void Path::operator/=(const Path& subDir) {

	if (subDir == ".")
		return;

	if (subDir == "/" || !subDir.isValid()) {
		return; /// \bug should this be exceptions?
	}

	if (subDir.m_path[0] != '/')
		m_path.push_back('/');

	m_path += subDir.m_path;

	// special linux handling, we do not want ./ in front of path
	if (m_path.find("./") == 0)  {
		assert(m_path.size() > 2);
		m_path = m_path.substr(2);
	}
}


void Path::operator/=(const std::string& subDir) {

	if (subDir.empty())
		return;

	// constructor removes trailing slashes but remind root dir
	Path subDirPath(subDir);

	if (subDirPath == "/" || subDirPath == "." ){
		return;
	}

	*this /= subDirPath;
}


void Path::addExtension( const std::string &myExt ) {

	std::string ext = myExt;
	trim(ext);

	if (ext.empty())
		return;

	if (ext.size() == 1 && ext[0] == '.')
		return;

	if (ext[0] != '.')
		ext = "." + ext;

	m_path += ext;
}


unsigned int Path::branchCount() const {

	if( m_path.empty())
		return 0;

	unsigned int count = 1;
	std::string::size_type pos = m_path.find('/');
	while(pos != std::string::npos) {
		if( pos < m_path.size() - 1)
			++count;
		pos = m_path.find('/' , pos + 1);
	}
	return count;
}



void Path::removeRelativeParts() {

	if(m_path.empty())
		return;

	bool isRootRelated = (m_path[0] == '/');

	bool hasRel = m_path.find("/../") != std::string::npos;

	// check if drive exists, path contains relative parts and has no placeholder
	if( !hasRel || !isAbsolute() ) {
		return;
	}

	std::vector<std::string> branches;
	explode(m_path, branches, "/");
	Path::removeRelativeParts(branches);
	// path not valid
	if(branches.empty()) {
		m_path.clear();
	}
	else {

		if ( isRootRelated ) {

			m_path.clear();
			for(std::vector<std::string>::iterator bit=branches.begin(); bit!=branches.end(); ++bit) {
				m_path += "/" + *bit;
			}

		} else {

			m_path = branches.front();
			for(std::vector<std::string>::iterator bit=branches.begin()+1; bit!=branches.end(); ++bit) {
				m_path += "/" + *bit;
			}

		}

	}
}


Path Path::subBranch(unsigned int begin, unsigned int count) const {

	bool isLinuxAbsolute = false;

	if( begin == 0 && count == 0)
		return *this;

	if (!isValid())
		throw std::runtime_error("Invalid path: '" + m_path + "'.");

	if ( m_path[0] == '/' ) {
		isLinuxAbsolute = true;
	}

	std::vector<std::string> branches;
	explode(m_path, branches, "/");
	if (isLinuxAbsolute){
		branches.insert( branches.begin(), "/" );
	}

	// do not forget root directory (Linux) or Root drive
	unsigned int size = branchCount();

	if ( count > size )
		throw std::runtime_error( "Insufficient branches for given count." );

	if ( begin > size-1 )
		throw std::runtime_error( "Illegal index for branch start." );

	// special case, append all branches after begin
	if (count == 0)
		count = size - begin;

	unsigned int end = begin + count;
	if (end > size)
		throw std::runtime_error( "Start index or count too large." );

	Path result;

	result = branches[begin];
	for( unsigned int i=begin+1; i<end; ++i) {
		result /= branches[i];
	}

	return result;
}


// static functions

bool Path::isRootPath(const Path& src) {

	Path cur = current();

	std::vector<std::string> currentBranches;
	std::vector<std::string> srcBranches;
	explode( cur.str(), currentBranches, "/" );
	explode( src.m_path, srcBranches, "/" );
	for(unsigned int i=0; i<currentBranches.size(); ++i) {
		if(i<srcBranches.size() && srcBranches[i] != currentBranches[i])
			return false;
	}
	return true;
}


bool Path::isRootPath( const Path& target, const Path& src ){

	std::vector<std::string> currentBranches;
	std::vector<std::string> srcBranches;
	explode( target.str(), currentBranches, "/" );
	explode( src.m_path, srcBranches, "/" );
	for(unsigned int i=0; i<currentBranches.size(); ++i) {
		if(i<srcBranches.size() && srcBranches[i] != currentBranches[i])
			return false;
	}
	return true;
}


Path Path::current() {

#if defined(_WIN32)
	wchar_t currentDir[FILENAME_MAX] = {0};
	if(! _wgetcwd(currentDir, FILENAME_MAX)) {
		switch(errno) {
			case ENOMEM: {
				throw std::runtime_error("Out of memory! Can not reserve " + FILENAME_MAX + " bytes.");
			}
			case ERANGE: {
				throw std::runtime_error("Path longer than " + FILENAME_MAX + " characters.");
			}
			default: {
				throw std::runtime_error("Couldn't deduce user directory." );
			}
		}

		return Path("");
	}
	try {
		return Path( WstringToUTF8(currentDir) );
	}
	catch(...) {
		throw std::runtime_error("Couldn't deduce user directory.");
	}
#elif __GNUC__
	char currentDir[FILENAME_MAX] = {0};
	if(!getcwd(currentDir, FILENAME_MAX)) {
		/// \todo some more error handling here
		throw std::runtime_error("Couldn't deduce user directory.");
	}
	return Path(currentDir);
#else
	#error Implement this for your compiler
#endif

}


Path Path::fromURI(std::string uripath) {

	if (uripath.find("file:/") != std::string::npos) {
		if (uripath.find("file:///") != std::string::npos) {
			uripath = uripath.substr(8);
		}
		else {
			uripath = uripath.substr(6);
		}
		trim(uripath);
		// if not on windows, prepend path with a slash
		if (uripath.size()>2) {
			if (uripath[1] != ':') {
				uripath = "/" + uripath;
			}
		}
	}
	return Path(uripath);
}


Path Path::operator+(const char * const str) const {
	std::string trimmedStr = str;
	trim(trimmedStr);
	return Path(m_path + trimmedStr);
}


Path Path::operator+(const std::string & str) const {
	std::string trimmedStr = str;
	trim(trimmedStr);
	return Path(m_path + trimmedStr);
}


#if defined(_WIN32)
Path::DriveInformation Path::driveType() const {

	if(m_path.size() != 2 || m_path[1] != ':')
		return DI_NO_DRIVE;

	// can be a existing drive
	// check if drive exist on system
	char targetPath[MAX_PATH] = {'\0'};
	DWORD res = QueryDosDeviceA(m_path.c_str(), targetPath, MAX_PATH);
	if(res == 0)
		return DI_UNKNOWN;
	unsigned int type = GetDriveTypeA(m_path.c_str());
	return static_cast<DriveInformation>(type);

}

bool Path::isDrive() const {
	DriveInformation dinfo = driveType();
	return dinfo >= DI_REMOVABLE && dinfo <= DI_RAMDISK;
}
#endif // defined(_WIN32)


/*******************************
 *
 * Protected Functions
 *
********************************/

void Path::removeRelativeParts(std::vector<std::string>& branches) {
	if(branches.empty())
		return;
	std::vector<std::string>::iterator it = std::find(branches.begin(), branches.end(), "..");
	// no relative parts found
	if(it == branches.end())
		return;
	// relative part found at beginning - path not valid or relative
	if(it == branches.begin()) {
		branches.clear();
		return;
	}
	branches.erase(it-1, it+1);
	removeRelativeParts(branches);
}


void Path::remove_trailing_slash(std::string& path) const {
	if (path.empty() || path.size() == 1)
		return;
	std::string::size_type pos1 = path.rfind('/');
	std::string::size_type pos2 = path.rfind('\\');
	std::string::size_type lastPos = path.size()-1;
	if (pos1 == lastPos || pos2 == lastPos) {
		path = path.substr(0, lastPos);
	}
}


std::string Path::remove_trailing_slash_copy(const std::string& path) const {
	if (path.empty())
		return path;
	std::string res = path;
	std::string::size_type pos1 = res.rfind('/');
	std::string::size_type pos2 = res.rfind('\\');
	std::string::size_type lastPos = res.size()-1;
	if (pos1 == lastPos || pos2 == lastPos) {
		res = res.substr(0, lastPos);
	}
	return res;
}


void Path::set(const std::string& path) {

	m_path = path;

	trim(m_path);

	if (!m_path.empty())
		std::replace(m_path.begin(), m_path.end(), '\\', '/');

	remove_trailing_slash(m_path);
	removeRelativeParts();

	// IBK::Path("./blub.txt") == "blub.txt"; //  (leading ./ is not stored)
	if ( m_path.size() > 2 && m_path[0] == '.' && m_path[1] == '/' ){
		m_path.replace( 0, 2, "");
	}

	// any first character followed by : must be upper case
	if ( m_path.size() >= 2 && m_path[1] == ':' ){
		m_path = firstCharToUpperUtf8(m_path);
	}

}

/*******************************
 *
 * Private Functions
 *
********************************/


Path::DirExistsResult Path::directoryExists(const std::string& dirname) const {

#ifdef _WIN32
	// create wstring from utf8-encoded string
	std::wstring direnameWStr = UTF8ToWstring(dirname);
	DWORD dwAttrib = GetFileAttributesW(direnameWStr.c_str());

	// neither directory nor file
	if(dwAttrib == INVALID_FILE_ATTRIBUTES) // INVALID_FILE_ATTRIBUTES is -1
		return NoSuchDirectory;

	// is directory
	if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
		return DirectoryExists;

	// must be a file
	return IsFile;

#elif __GNUC__
	struct stat buf;
	std::string cleanPath = remove_trailing_slash_copy(dirname);
	int res = stat(cleanPath.c_str(), &buf);
	if (res == -1) {
		switch (errno) {
			case EACCES :
				throw std::runtime_error( "Cannot access file/directory '" + dirname + "'!");
			default: break;
		}
		return NoSuchDirectory;
	}
	// check if it is a directory
	if (S_ISDIR(buf.st_mode))
		return DirectoryExists;

	return IsFile;

#else
#error "Implement this for your compiler!"
#endif
}


std::string Path::firstCharToUpperUtf8(const std::string& orig) const {

	std::string retVal;
	retVal.reserve(orig.size());
	std::mbstate_t state = std::mbstate_t();
	std::vector<char> buf(MB_CUR_MAX + 1, '\0');
	size_t i = 0;
	if (orig.size() > 0) {
		if (orig[i] > 0) {
			retVal += static_cast<char>(toupper(orig[i]));
			++i;
		} else {
			wchar_t wChar;
			size_t len = std::mbrtowc(&wChar, &orig[i], MB_CUR_MAX, &state);
			// If this assertion fails, there is an invalid multi-byte character.
			// However, this usually means that the locale is not utf8.
			// Note that the default locale is always C. Main classes need to set them
			// To utf8, even if the system's default is utf8 already.
			assert(len > 0 && len <= static_cast<size_t>(MB_CUR_MAX));
			i += len;
			size_t ret = std::wcrtomb(&buf[0], towupper(wChar), &state);
			assert(ret > 0 && ret <= static_cast<size_t>(MB_CUR_MAX));
			buf[ret] = 0;
			retVal += &buf[0];
		}
	}
	for (; i < orig.size(); ++i) {
		retVal += orig[i];
	}
	return retVal;
}


/*******************************
 *
 * Global Functions
 *
********************************/

const Path operator/(const Path& lhs, const Path& rhs) {
	Path result = lhs;
	result /= rhs;
	return result;
}

const Path operator/(const Path& lhs, const std::string& rhs) {
	Path result = lhs;
	result /= Path(rhs);
	return result;
}

const Path operator/(const std::string& lhs, const Path& rhs) {
	Path result(lhs);
	result /= rhs;
	return result;
}

