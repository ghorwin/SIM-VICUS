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

#ifndef IBK_PathH
#define IBK_PathH

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

#include "IBK_StringUtils.h"

namespace IBK {

/*! \brief Class contains a directory or a file with its path (relative or absolute).

	It contains also functions for path manipulating or checks.
	Path names are expected to be UTF8 encoded. This is actually only needed on WIN32 when
	using wstring-functions or WINAPI functions.

	Path names may contain path placeholders:
	\code
	IBK::Path p("${Project Directory}/relative_dir/file.txt");
	\endcode

	** Functions for storing/retrieving paths **

	\code
	// constructors
	IBK::Path p; // empty path
	IBK::Path p2("c:\\blubb\\file.txt"); // from c-string
	std::string relPathString("../../blubb/file.txt");
	IBK::Path p3(relPathString); // from std::string
	IBK::Path p4(p3); // copy constructor

	p3.clear(); // clearing Path
	p3 = p4; // assignment of IBK::Path to IBK::Path
	p3 = relPathString; // assignment of std::string to IBK::Path

	p3.swap(p4); // swapping Paths

	// returning strings
	std::string pathStr = p4.str();
	std::wstring pathWStr = p4.wstr(); // only _WIN32

	std::string OS_string = p3.osStr(); // -> "..\\..\\blubb\\file.txt" on Windows
	\endcode


	Internally path separators are stored as / characters. A trailing slash is never stored,
	an empty string corresponds to root path on Linux/Unix/Mac. On Windows, the drive
	letter and colon identify the root path ("D:"). Driver letters are converted to upper-case
	when setting path.

	An empty Path (IBK::Path("")) is invalid. Most query functions will throw an exception.

	Leading and trailing whitespaces are automatically removed by all member functions. This implies
	that a valid path may not consist of whitespaces (tabs and spaces) only!
*/
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

	/*! Constructor for zero terminated char arrays.
		Is used for creating a Path instance from a char*.
		The path is expected in UTF8 encoding.
	*/
	explicit Path(const char * const path);

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

	/*! Swap function with nothrow guarantee.*/
	void swap(Path& src);

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

	/*! Returns true if the path points to an _existing_ file.
		If Path is relative, the absolute path will first be retrieved used
		current working directory.
		\return Returns false also if file does not exist.
	*/
	bool isFile() const;

	/*! Returns true if the path points to an _existing_ directory.
		If Path is relative, the absolute path will first be retrieved used
		current working directory.
		\return Returns false also if directory does not exist.
	*/
	bool isDirectory() const;

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

	/*! Returns extension of path (everything after the last dot).
		We only consider one extension, that is everything after the last dot in the file path,
		not including the dot.
		\code
			IBK::Path("../blub.txt").extension() --> "txt"
			IBK::Path("../archive.tar.gz").extension() --> "gz"
			IBK::Path("../README").extension() --> ""
			IBK::Path("..").extension() --> "" // no extension on relative paths
			IBK::Path(".").extension() --> "" // no extension on relative paths
			IBK::Path("C:").extension() --> "" // no extension on root paths
			IBK::Path("/").extension() --> "" // no extension on root paths
		\endcode
		\sa withoutExtension()
		\sa addPath()
	*/
	std::string extension() const;

	/*! Returns extension of path (everything after the last dot).
		We only consider one extension, that is everything after the last dot in the file path,
		not including the dot.
		\code
			IBK::Path("../blub.txt").withoutExtension() --> "../blub"
			IBK::Path("C:\\blubb\\bla\\blub").withoutExtension() --> "C:/blubb/bla/blub"
			IBK::Path("./archive.tar.gz").withoutExtension() --> "archive.tar"
		\endcode
		\sa addPath()
		\sa extension()
	*/
	IBK::Path withoutExtension() const;


	// ** Modification Routines **

	/*! Appends another path to this path.
		This function always adds a / and afterwards the other path.
		\code
			IBK::Path("../bla").addPath( IBK::Path("blubb/blo.txt") )	// --> "../bla/blubb/blo.txt"
			IBK::Path("../bla").addPath( IBK::Path("") )				// --> "../bla"
			IBK::Path("../bla").addPath( IBK::Path("/") )				// --> "../bla"
			IBK::Path("../bla").addPath( IBK::Path("./") )				// --> "../bla"
			IBK::Path("../bla").addPath( IBK::Path("/blub") )			// --> "../bla/blub"
			IBK::Path("../bla").addPath( IBK::Path("C:\\blub") )		// --> "../bla/C:/blub" -> manual Win32 check
			IBK::Path("../bla.txt").addPath( IBK::Path("blub") )		// --> "../bla.txt/blub"

			// is this correct?
			IBK::Path("").addPath( IBK::Path("D:/test.txt") )			// --> "D:/test.txt"
		\endcode

		\sa operator/=()(const IBK::Path & subDir)
		\sa withoutExtension()
		\sa extension()
	*/
	void addPath(const IBK::Path & subDir);

	/*! Appends another path.
		\sa addPath()
	*/
	void operator/=(const Path& subDir);


	/*! Appends another path.
		\sa addPath()
	*/
	void operator/=(const std::string& subDir);

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

	/*! Checks if the path contains placeholders in format ${xxx}. */
	bool hasPlaceholder() const;

	/*! Returns the placeholder string if one exists.
		\code
			IBK::Path("${Project Directory}\subdir\file.txt").placeholderString() // --> "Project Directory"
		\endcode
	*/
	std::string placeholderString() const;

	/*! Tries to exchange existing placeholders by path from the given map.
		If the path doesn't contain a placeholder or the existing placeholder cannot be found in the map
		the returning path is equal to the existing.
		\code
			IBK::Path path("${my path}/fname.txt");
			std::map<string,IBK::Path> placeholders;
			placeholders["my path"] = "C:\\data";
			// we call replace_path_placeholders to get the full path
			IBK::Path fullpath = path.withReplacedPlaceholders(placeholders);
			// fullpath is now "C:/data/fname.txt"
			// noop test
			IBK::Path path2 = "D:/data/file.txt";
			IBK::Path fullpath2 = path2.withReplacedPlaceholders(placeholders);
			// fullpath2 is now still "D:/data/file.txt"

			// recursive placeholder substitution
			placeholders["Project Directory"] = "D:/projects";
			placeholders["Materials"] = "${Project Directory}/materials";

			IBK::Path path3 = "${Materials}/mat1.m6";
			IBK::Path fullpath3 = path3.withReplacedPlaceholders(placeholders);
			// fullpath3 -> "D:/projects/materials/mat1.m6"
		\endcode
	*/
	Path withReplacedPlaceholders(const std::map<std::string, Path> & placeHoldersMap) const;

	/*! Convenience function, used to extract path placeholders in {xxx} format (placeholder is enclosed in curly braces).
		\param placeholderName Here the placeholder will be stored in.
		\param relativePathPart The filepath after the placeholder.
		\return Returns true, if placeholder could be extracted.

		Example:
		\code
			IBK::Path fnameWithPlaceholder("${User Data Dir}/Materials/mat_01.mat");
			IBK::Path relativeFname;
			std::string placeholderName;
			bool success = fnameWithPlaceholder.extractPlaceholder(placeholderName, relativeFname);
			// success = true
			// relativeFname = "Materials/mat_01.mat"
			// placeholderName = "User Data Dir"    (the curly braces are removed)
		\endcode

		\note If the string does not contain a path placeholder, the full path name is stored in fname and placeholder
			  will be empty.
	*/
	bool extractPlaceholder(std::string & placeholderName, IBK::Path & relativePathPart) const;


	/*! Attempts to replace a part of the path with a placeholder path.

		Example:
		\code
			IBK::Path placeholderPath("/home/user/data");
			std::string placeholderName = "User Data";
			IBK::Path fullPath("/home/user/data/myfiles/datafile.txt");
			bool success = fullPath.insertPlaceholder(placeholderName, placeholderPath);
			// success = true
			// fullPath = "${User Data}/myfiles/datafile.txt";
		\endcode
	*/
	bool insertPlaceholder(const std::string & placeholderName, const IBK::Path & placeholderPath);


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

	/*! Returns the size of the file in bytes then the given path is a file otherwise it returns -1.*/
	int64_t fileSize() const;

	/*! Return the last modification time for the given file or directory as time_t struct.*/
	std::time_t lastWriteTime() const;

	/*! Return the last write time for the given file as time_t struct.*/
	std::time_t fileTime() const;

	/*! Modifies path name such that it is compatible with FAT32 file systems (Windows).
		Basically, all occurrances of : & and other special characters are replaced by _.
	*/
	void makeFatCompatible();


	// ** static functions **


	/*! Set the file date to given date and time
		\param filename Filename with path (must exist).
		\param hour Hour from 0 to 23
		\param minute Minute from 0 to 59.
		\param second Second divided by 2.
		\param day Day of the month from 1 to 31 depending on month.
		\param month Month in the year from 1 to 12.
		\param year Year as value (valid from 1601 to 30827).
	*/
	static bool setFileTime(	const IBK::Path& filename,
								int hour,
								int minute,
								int second,
								int day,
								int month,
								int year);

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

	/*! Set the current directory.
		\return true if directory exists and is set as current dir, false otherwise.
		\code
			IBK::Path cdPath("C:\\home\\ghorwin");
			IBK::Path::setCurrent(cdPath);
		\endcode
	*/
	static bool setCurrent(const IBK::Path & p);

	/*! Creates full path including all required parent paths.
		\return True if directory was created or existed already, or false if
				directory could not be created (either path existed as file, for invalid path,
				or access denied).
	*/
	static bool makePath(const IBK::Path & p);

	/*! Removes path and all its subdirectories/files. If current directory is with in the path to be deleted
		the current directory is set to the parent path of p \sa parentPath(). This behaviour prevents OS warnings.
		However, p will be deleted if OS access rights are sufficient enough.
		\param p Path or file to be removed
		\param quiet If true no error message will come in case of errors
		\code
			IBK::Path::remove("/home/mypath/Documents"); // -> removes subdirectory Documents and its content
		\endcode
		\warning This function is very very very unsafe because on Linux/mac it uses std::system() to
			evaluate the 'rm -rf' command. Bad things can happen to the user's home directory, if an
			invalid path is passed.
				Example: "/home/ghorwin" ->
				Delphin project file created "/home/ghorwin.d6p"->
				Run simulation -> and *boom*
	*/
	static bool remove(const IBK::Path & p, bool quiet = false);

	/*! Copy file/directory identified by 'source' to 'target', in cases of directories uses recursive copy.
		\bug If target file exists, it should be overwritten or at least some error handling should be implemented. Test!
		If target directory exists, the new copied files are merged with existing files in the directory.
		\code
			IBK::Path::copy("/tmp/blubb.txt", "/tmp/bla.txt");
			// same as "cp blubb.txt bla.txt" in /tmp

			IBK::Path::copy("/tmp1/blubb.txt", "/tmp2/blubb.txt");
			// copy file "blubb.txt" from /tmp1 to /tmp2
			// target will be overwritten if existing

			IBK::Path::copy("/tmp/blubb.txt", "/tmp/blubbs");
			// 'blubbs' is a directory -> "cp blubb.txt blubbs" -> "/tmp/blubbs/blubb.txt"

			IBK::Path::copy("/tmp/blubbs", "/tmp/blubb.txt");
			// 'blubbs' is a directory, 'blubb.txt' must also be a directory, otherwise function returns false
		\endcode
	*/
	static bool copy(const IBK::Path & source, const IBK::Path & target);

	/*! Moves file/directory identified by 'source' to 'target', in cases of directories works recoursively.
		\code
			IBK::Path::move("/tmp/blubb.txt", "/tmp/bla.txt");
			// same as "mv blubb.txt bla.txt" in /tmp

			IBK::Path::move("/tmp/blubb.txt", "/tmp/blubbs");
			// 'blubbs' is a directory -> "mv blubb.txt blubbs" -> "/tmp/blubbs/blubb.txt"

			IBK::Path::move("/tmp/blubbs", "/tmp/blah");
			// 'blubbs' and 'blubb' are a directories, blubb doesn't exist -> "/tmp/blah"

			IBK::Path::move("/tmp/blubbs", "/tmp/blah");
			// 'blubbs' and 'blah' are a directories, blubb exists -> "/tmp/blah/blubbs"

			IBK::Path::move("/tmp/blubbs", "/tmp/blubb.txt");
			// 'blubbs' is a directory, 'blubb.txt' must also be a directory, otherwise function returns false
		\endcode
	*/
	static bool move(const IBK::Path & source, const IBK::Path & target);

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

	/*! Compares the two paths via their string names.*/
	bool operator<(const Path& other) const {
		return str() < other.str();
	}

	/*! Write the path to a stream as text.*/
	friend std::ostream& operator<<(std::ostream& out, const Path& val) {
		out << val.m_path;
		return out;
	}

	/*! Read the path from a stream as text.*/
	friend std::istream& operator>>(std::istream& in, Path& val) {
		std::string pstr;
		in >> pstr;
		val.set(pstr);
		return in;
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

} // namespace IBK

/*! \file IBK_Path.h
	\brief Declaration of class Path, a representation of a file path or file name.
*/

#endif // IBK_PathH
