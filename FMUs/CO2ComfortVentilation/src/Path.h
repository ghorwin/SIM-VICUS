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

#ifndef PathH
#define PathH

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#else
	#include <stdint.h>
#endif

#include <string>
#include <map>
#include <iosfwd>
#include <ctime>

#include "Helpers.h"

class Path {
public:
	/*! Standard constructor.*/
	Path();

	/*! Constructor for strings.
		Is used for creating a Path instance from a std::string.
		Internally, path simplifies path representation in some cases
		\code
			IBK::Path("") == ""; // (invalid path)
			IBK::Path("./blub.txt") == "blub.txt"; //  (leading ./ is not stored)
			IBK::Path("blub.txt") == "blub.txt";
			IBK::Path("../blub.txt") == "../blub.txt";
			IBK::Path("c:\\blubb\\bla\\blub") == "C:/blubb/bla/blub""; // c: -> C:
			IBK::Path("C:\") == "C:";
			IBK::Path("/") == "/";
			IBK::Path("./") == ".";
			IBK::Path("../") == "..";
			IBK::Path("/..") == "/.."; // (invalid path)
		\endcode
	*/
	explicit Path(const std::string & path);

	/*! Copy assignment operator for strings.
		The path is expected in UTF8 encoding.
	*/
	Path& operator=(const std::string & path);

	/*! Copy assignment operator for strings.
		The path is expected in UTF8 encoding.
	*/
	Path& operator=(const char * const path);

	/*! Clears the content (creates an invalid/empty path).*/
	void clear();

	/*! Returns the current path as string.
		The returning string is UTF8 encoded.
		\note Use this function to retrieve a path representation suitable for serialization.
			  An IBK::Path can be reconstructed from this string.
		\warning Using str().c_str() may cause undefined behavior with some compilers like VC.
		\code
		IBK::Path p;

		const char * s = p.str().c_str(); // looks ok, but can be bad
		// p.str() returns an unnamed copy of the internal string say x
		// x.c_str() returns a pointer to the memory occupied by this unnamed pointer
		// when you now copy this pointer and do NOT evaluate it directly (e.g. in a string constructor)
		// the temporary string x goes out of scope and you access released memory

		\endcode
	*/
	std::string str() const;

	/*! Returns a pointer to the start of the internal string representation.
		This pointer is valid as long as the path is not modified.
		The filename is UTF8 encoded.
	*/
	const char * c_str() const { return &m_path[0]; }

#if defined(_WIN32)
	/*! Returns the current path as wstring.
		\note If you need a raw pointer to this string, make sure to keep the copy of this string
		as long as you need the pointer.
	*/
	std::wstring wstr() const;

	/*! Returns the current path as string.
		Furthermore the standard path dividers will be exchanged
		by the ones used natively from operating system.*/
	std::wstring wstrOS() const;
#endif

	/*! Returns the current path as string.
		Furthermore the standard path dividers will be exchanged
		by the ones used natively from operating system.
		The returning string is UTF8 encoded.
	*/
	std::string osStr() const;

	/*! Removes relative parts if possible.
		A leading . or .. will be replaced by using current directory.
	*/
	Path absolutePath() const;

	/*! Check basic requierements for creating a relativ path from current one to given path.
	   Checks:
	   - current and given path is valid
	   - absolut paths from current and given path are valid
	   - drive of current and given path is the same (only Windows)
	*/
	bool canCreateRelativePath(const Path& toPath, std::string& errstr) const;

	/*! Tries to transform the currently stored path into a relative path
		to the path passed as argument.
		Returns an empty/invalid path if the path itself (this object) is empty.
		\code
			Path("/home/myfiles/data/datfile.txt").relativePath("/home/myfiles/otherfiles") == "../data/datfile.txt";
			Path("bla/blub.txt").relativePath("bla"); == "blub.txt";
			Path("").relativePath("/blubb") --> Exception
			Path(".").relativePath("file.txt") --> ".."
			Path("file.txt").relativePath(".") --> "file.txt"
			Path("bla/blubber").relativePath("bla/blub"); == "../blubber";
		\endcode
		Throws an IBK::Exception if path or toPath is invalid or relative path cannot be created.
	*/
	Path relativePath(const Path& toPath) const;

	/*! \brief Returns the parent path if one exists.
		In case of relative path and only one branch no parent path exists and
		return value will be same as given path.
		In order to avoid this call absolutePath() first.
		\code
			IBK::Path("").parentPath()						// --> exception
			IBK::Path("./blub.txt").parentPath()			// --> "."
			IBK::Path("blub.txt").parentPath()				// --> "."
			IBK::Path("../blub.txt").parentPath()			// --> ".."
			IBK::Path("C:\\blubb\\bla\\blub").parentPath()	// --> "C:/blubb/bla"
			IBK::Path("C:").parentPath()					// --> exception
		\endcode
	*/
	Path parentPath() const;

	/*! Returns true if path holds an non-empty string.
		\code
		// equivalent to
		bool valid = !m_path.empty();
		\endcode
		\todo what about a string only holding whitespaces?
	*/
	bool isValid() const;

	/*! Returns true if the directory or file exists.
		If Path is relative, the absolute path will first be retrieved used
		current working directory.
	*/
	bool exists() const;

	/*! Returns true if the path is an absolute path
		and false if the path is a relative one.
		It cannot resolve paths in URI or UNC notation. */
	bool isAbsolute() const;

	/*! Returns the filename/last part of the path including its extension.
		\code
			IBK::Path("").filename() --> exception
			IBK::Path("./blub.txt").filename() --> "blub.txt"
			IBK::Path("blub.txt").filename() --> "blub.txt"
			IBK::Path("../blub.txt").filename() --> "blub.txt"
			IBK::Path("C:\\blubb\\bla\\blub").filename() --> "blub"
			IBK::Path("C:").filename() --> ""  (weil "C:" == "C:\\" == "C:/" == "C:" + "/" + "")
		\endcode
	*/
	Path filename() const;


	/*! Adds an extension.
		The current path will be changed, but the appended
		extension is trimmed to IBK rules.
		\code
			IBK::Path("blub").addExtension("txt"); // --> blub.txt
			IBK::Path("blub.bla").addExtension("txt"); // --> blub.bla.txt
			IBK::Path("blub").addExtension(".txt"); // --> blub.txt
			IBK::Path("blub").addExtension(""); // --> blub
			IBK::Path("blub").addExtension("."); // --> blub
			IBK::Path("blub").addExtension(" "); // --> blub
			IBK::Path("blub").addExtension(".long.extension"); // --> blub.long.extension
		\endcode
	*/
	void addExtension( const std::string& ext);

	// ** Modification Routines **

	/*! Appends another path.
		\sa addPath()
	*/
	void operator/=(const Path& subDir);


	/*! Appends another path.
		\sa addPath()
	*/
	void operator/=(const std::string& subDir);

	/*! Returns the number of branches in the path.

		/// \todo example should return 2 otherwise we need to check consistency with drive definition of /?
		For example:
		\code
		d:/test1/temp	returns 3
		\endcode
	*/
	unsigned int branchCount() const;

	/*! Removes relative parts in path.
		Called from constructors and from addPath().
		\code
		IBK::Path fname0( "/home/stvo/ibk/src/../Materials/mat_01.mat" );
		IBK::Path fname1("../Materials/mat_01.mat");
		IBK::Path fname2("/../Materials/mat_01.mat");
		IBK::Path fname3("./../Materials/");
		fname0.removeRelativeParts();
		fname1.removeRelativeParts();
		fname2.removeRelativeParts();
		fname3.removeRelativeParts();
		// fname0 == "/home/stvo/ibk/Materials/mat_01.mat"
		// fname1 == "../Materials/mat_01.mat"
		// fname2 == "/../Materials/mat_01.mat"
		// fname3 == "../Materials"
		\endcode
	*/
	void removeRelativeParts();

	/*! Creates a new path from given range of branches of the current path.
		\param begin Begin index. Should be lower than branchCount - 1
		\param count Number of branches including this on index begin. If count == 0 we append all elements after begin.
		For example:
		\code
		IBK::Path p("d:/programs/IBK/Delphin/Delphin5/Climate_DB").subBranch(1, 3) == IBK::Path("programs/IBK/Delphin);
		IBK::Path p("/home/stvo/test").subBranch(0,0) == IBK::Path("/home/stvo/test");
		IBK::Path p("/home/stvo/test").subBranch(0,1) == IBK::Path("/");
		IBK::Path p("/home/stvo/test").subBranch(6,0) -> exception
		IBK::Path p("/home/stvo/test").subBranch(0,6) -> exception
		IBK::Path p("").subBranch(0,1) -> exception
		\endcode
	*/
	Path subBranch(unsigned int begin, unsigned int count = 0) const;

	/*! Returns the name of the drive if one exists.
		This works only for absolute paths under Windows.
		In all other cases it returns an empty string.
		UNC pathes are not supported.
		\code
		drive("D:/test"); // returns "D:"
		drive("../test"); // returns ""
		drive("/test"); // returns ""
		\endcode
	*/
	std::string drive() const;

	// ** static functions **


	/*! Returns true if the path given by \a src is a root path of the current path.
		For example:
		\code
		current path d:/programs/IBK/Delphin/Delphin5/Climate_DB
		isRootPath(Path("d:/programs/IBK")) returns true
		isRootPath(Path("d:/programs/test")) returns false
		\endcode
	*/
	static bool isRootPath(const Path& src);

	/*! Returns true if the path given by \a src is a root path of \a target path.
		\code
		isRootPath(Path("d:/programs/IBK/Delphin/Delphin5/Climate_DB"), Path("d:/programs/IBK")) returns true
		isRootPath(Path("d:/programs/IBK/Delphin/Delphin5/Climate_DB"), Path("d:/programs/test")) returns false
		\endcode
	*/
	static bool isRootPath( const Path& target, const Path& src );

	/*! Returns current working directory. */
	static Path current();

	/*! Converts a file-based URI to path.
		\code
		// Windows
		IBK::Path p = IBK::Path::fromURI("file:///C:/tmp/somefile.txt"); --> "C:/tmp/somefile.txt"
		IBK::Path p = IBK::Path::fromURI("file:/C:/tmp/somefile.txt"); --> "C:/tmp/somefile.txt"
		// Linux
		IBK::Path p = IBK::Path::fromURI("file:///tmp/somefile.txt"); --> "/tmp/somefile.txt"
		IBK::Path p = IBK::Path::fromURI("file:///a"); --> "/a"
		\endcode
		\note Argument is passed by value because string needs to be modified inside function.
	*/
	static Path fromURI(std::string uripath);

	// Operators

	/*! Compares the two paths by just comparing there string represantation.*/
	bool operator==(const Path& other) const {
		return other.m_path == m_path;
	}

	/*! Compares the two paths by just comparing there string represantation.*/
	bool operator!=(const Path& other) const {
		return other.m_path != m_path;
	}

	/*! Adds a string to the path.
		This is a convenience function and replaces
		\code
		newpath = IBK::Path(path.str() + IBK::trim_copy(str));
		\endcode
	*/
	Path operator+(const char * const str) const;

	/*! Adds a string to the path.
		This is a convenience function and replaces
		\code
		newpath = IBK::Path(path.str() + IBK::trim_copy(str));
		\endcode
	*/
	Path operator+(const std::string & str) const;

protected:
#if defined(_WIN32)
	enum DriveInformation {
		DI_UNKNOWN,
		DI_NO_ROOT_DIR,
		DI_REMOVABLE,
		DI_FIXED,
		DI_REMOTE,
		DI_CDROM,
		DI_RAMDISK,
		DI_NO_DRIVE
	};

	/*! Returns the drive type in case the path represents a drive.
		This is only useful for Windows OS.
		In case of other operating systems it returns NO_WINDOWS.*/
	DriveInformation driveType() const;

	/*! Returns true if the given path represents a valid drive on a Windows system othwerwise false.
		For more information see driveType().*/
	bool isDrive() const;
#endif // defined(_WIN32)

protected:

	/*! Helper function for user in removeRelativeParts().*/
	static void removeRelativeParts(std::vector<std::string>& branches);

	/*! Removes any trailing slashes or backslashes from file path. */
	void remove_trailing_slash(std::string& path) const;

	/*! Removes any leading slashes or backslashes from file path and returns the result. */
	std::string remove_trailing_slash_copy(const std::string& path) const;

	/*!	Set the internal path, trims it and replaces
		backslashes by slashes, relative path parts, a leading local path
		and trailing slashes are removed. Windows drive letters are converted
		to upper case letters.
	*/
	void set(const std::string& path);

	/*! Internal string representation of path.
		UTF8 encoded, other rules see description of IBK::Path.
	*/
	std::string	m_path;

private:
	/*! Possible return values for the directory_exists() function. */
	enum DirExistsResult {
		DirectoryExists,
		IsFile,
		NoSuchDirectory
	};

	/*! Tests if a directory exists.
		\param dirname The full path name in posix format (no backslashes) UTF8 encoded.
		\return Returns IBK::DirectoryExists if directory exists and can be accessed or returns
				IBK::NoSuchDirectory if directory does not exist or cannot be accessed or conversion to unicode fails.
				or returns IBK::IsFile if dirname points to a file.
	*/
	DirExistsResult directoryExists(const std::string& dirname) const;

	/*! Converts the first character of a string to upper case UTF8.
		\code
			setlocale(LC_CTYPE, "en_US.utf8");
			IBK_ASSERT("Foo" == firstCharToUpperUtf8("foo"));
			IBK_ASSERT("Foo" == firstCharToUpperUtf8("Foo"));
			IBK_ASSERT("#foo" == firstCharToUpperUtf8("#foo"));
			IBK_ASSERT("ßfoo" == firstCharToUpperUtf8("ßfoo"));
			IBK_ASSERT("Éfoo" == firstCharToUpperUtf8("éfoo"));
			IBK_ASSERT("Éfoo" == firstCharToUpperUtf8("Éfoo"));
		\endcode
	*/
	std::string firstCharToUpperUtf8(const std::string& orig) const;

};

//#ifndef _WIN32
/*! Adds the given subdirectory or filename.*/
const Path operator/(const Path& lhs, const Path& rhs);

/*! Adds the given subdirectory or filename.*/
const Path operator/(const Path& lhs, const std::string& rhs);

/*! Adds the given subdirectory or filename.*/
const Path operator/(const std::string& lhs, const Path& rhs);

//#endif // _WIN32

/*! Compares the two paths by just comparing their string representation, const char version.*/
inline bool operator==(const Path& lhs, const char * const rhs) { return lhs.str() == rhs; }

/*! Compares the two paths by just comparing their string representation, const char version.*/
inline bool operator!=(const Path& lhs, const char * const rhs) { return lhs.str() != rhs; }

/*! Compares the two paths by just comparing their string representation, std::string version.*/
inline bool operator==(const Path& lhs, const std::string & rhs) { return lhs.str() == rhs; }

/*! Compares the two paths by just comparing their string representation, std::string version.*/
inline bool operator!=(const Path& lhs, const std::string & rhs) { return lhs.str() != rhs; }

#endif // IBK_PathH
