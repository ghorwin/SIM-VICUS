// Keywordlist-creator-tool
//
// Author:			Andreas Nicolai <andreas.nicolai@tu-dresden.de>
// Extended by:		Stefan Vogelsang <Stefan.Vogelsang@tu-dresden.de>
//					http://www.bauklimatik-dresden.de/delphin
//					supported format:
//					// Keyword:	Blub			[m3]					{0.3}									<#FFFFFF>				'Description'
//								KeywordName		KeywordDefaultUnit		KeywordDefaultValue(in DefaultUnit)		KeywordColor			DescriptionOfKeyword
//					ColorIsDefined as #RRGGBB, each of R, G, and B is a single hex digit
//
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <map>

#ifdef WIN32

#undef UNICODE
#include <cstdlib>
#include <cstdio>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>
#include <cstring>

#else // WIN32

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <cstdio>
#include <map>

#endif // WIN32



struct Keyword {
	std::string		category;			// The enumeration type (full C++ type including class prefix)
	std::string		keyword;			// The keyword as string
	std::string		description;		// The english description text (optional)
	std::string		unit;				// The default unit (if any, otherwise an empty string)
	double			defaultValue;		// The default value in default unit if any
	int				index;				// The index/enumeration number
	int				maxIndexInCategory; // Category related maximum index
	std::string		color;				// Color assigned to this keyword; default color is black #FFFFFF
};

// Prototypes
double string2val(const std::string& str);
Keyword parse_keyword_line(const std::string & line, std::map< std::string, int > mapReplacementsToInt );
bool listHeaders(const std::string & dir, std::vector<std::string> & files);
bool file_exists(const std::string& dirname);
void trim(std::string& str);
std::string stripSpaces(const std::string & input);
void trimLastBracket(std::string& str);
bool parse_headers(const std::string& src_dir, const std::vector<std::string>& hfiles, std::vector<Keyword>& keywordlist );
bool write_keyword_data(const std::string& output_file, const std::vector<Keyword>& keywordlist);
bool generate_keywordlist_code(const std::string& output_file, const std::vector<Keyword>& keywordlist );
bool generate_header_code(const std::string& output_file, bool isQt);
bool generate_keywordlist_qt(const std::string& output_file, const std::vector<Keyword>& keywordlist );

std::string CPP_HEADER;
std::string NAMESPACE;
std::string HEADER;


std::string QT_HEADER;
std::string QT_CPP_HEADER;
std::string QT_CPP_FOOTER;

const char * const QT_HEADER_TEMPLATE =

"#ifndef ${PREFIX}_KeywordListQtH\n"
"#define ${PREFIX}_KeywordListQtH\n\n"
"#include <${PREFIX}_KeywordList.h>\n"
"#include <QString>\n"
"#include <QColor>\n"
"#include <QCoreApplication>\n\n"
"namespace ${NAMESPACE} {\n\n"
"class KeywordListQt {\n"
"Q_DECLARE_TR_FUNCTIONS(KeywordListQt)\n"
"public:\n\n"
"	KeywordListQt();\n\n"
"	/*! Returns a keyword for an enum value t of type enumtype. */\n"
"	static const char * Keyword(const char * const enumtype, int t){ return KeywordList::Keyword( enumtype, t ); }\n\n"
"	/*! Returns a formate keyword string as in \"Keyword [Unit]\" for an enum value t of type enumtype. */\n"
"	static QString FormatedKeyword(const char * const enumtype, int t){ return QString(\"%1 [%2]\").arg( KeywordList::Keyword( enumtype, t ) ).arg(KeywordList::Unit( enumtype, t ) ); } \n\n"
"	/*! Returns a translated description for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		If no descrption is given, the keyword itself is returned.\n"
"		\\param enumtype 		The full enumeration type including the class name.\n"
"		\\param t 				The enumeration type cast in an int.\n"
"		\\param no_description	The optional argument is set to true, if there was no description\n"
"		for this keyword, otherwise to false.\n"
"	*/\n"
"	static QString Description(const std::string & category, unsigned int keywordId);\n\n"
"	/*! Returns a default unit for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		Returns an empty string if no default unit was specified.\n"
"		\\param enumtype The full enumeration type including the class name.\n"
"		\\param t 		The enumeration type cast in an int.\n"
"	*/\n"
"	static const char * Unit(const char * const enumtype, int t){ return KeywordList::Unit( enumtype, t ); }\n\n"
"	/*! Returns a default unit for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		Returns an empty string if no default unit was specified.\n"
"		\\param enumtype The full enumeration type including the class name.\n"
"		\\param t 		The enumeration type cast in an int.\n"
"	*/\n"
"	static QColor Color(const char * const enumtype, int t){ return QColor( KeywordList::Color( enumtype, t ) ); }\n\n"
"	/*! Returns a default value for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		Returns an nan if no default value was specified.\n"
"		\\param enumtype The full enumeration type including the class name.\n"
"		\\param t 		The enumeration type cast in an int.\n"
"	*/\n"
"	static double DefaultValue(const char * const enumtype, int t){ return KeywordList::DefaultValue( enumtype, t ); }\n\n"
"	/*! Returns an enumeration value for a given keyword kw of type enumtype.\n"
"		This function throws an exception if the keyword or the enumeration type is invalid or unknown.\n"
"		\\param enumtype 	The full enumeration type including the class name.\n"
"		\\param kw 			The keyword string.\n"
"		\\param deprecated 	The optional argument is set the true if the keyword kw is deprecated.\n"
"	*/\n"
"	static int Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated = NULL){ return KeywordList::Enumeration( enumtype, kw, deprecated ); }\n\n\n"
"	/*!	Returns the maximum index for entries of a category in the keyword list.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		\\param enumtype 	The full enumeration type including the class name.\n"
"	*/\n"
"	static int MaxIndex(const char * const enumtype){ return KeywordList::MaxIndex( enumtype ); }\n\n\n"
"	/*! Returns the number of keywords in this category.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		\\param enumtype 	The full enumeration type including the class name.\n"
"	*/\n"
"	static unsigned int Count(const char * const enumtype){ return KeywordList::Count( enumtype ); }\n\n\n"
"	/*! Checks whether a keyword exists in the enumeration of type enumtype.\n"
"		\\return Returns true if the keyword is valid, otherwise false.\n"
"	*/\n"
"	static bool KeywordExists(const char * const enumtype, const std::string & kw){ return KeywordList::KeywordExists( enumtype, kw ); }\n\n\n"
"	/*! Checks whether a category of type enumtype exists.\n"
"		\\return Returns true if the category/enum type exists, otherwise false.\n"
"	*/\n"
"	static bool CategoryExists(const char * const enumtype){ return KeywordList::CategoryExists( enumtype ); }\n\n\n"

"}; // class\n\n"
"} // namespace \n"
"/*!\n"
"	\\file ${PREFIX}_KeywordList.h\n"
"	\\brief Contains the declaration of class KeywordListQt.\n"
"*/\n\n"
"#endif // ${PREFIX}_KeywordListQtH\n";



const char * const QT_CPP_HEADER_TEMPLATE =

"#include \"${PREFIX}_KeywordListQt.h\"\n\n"
"namespace ${NAMESPACE} {\n"
"KeywordListQt::KeywordListQt() {\n\n";

const char * const QT_CPP_FOOTER_TEMPLATE =

"}\n\n\n"
"QString KeywordListQt::Description( const std::string & category, unsigned int keywordId) { \n\n"
"	std::string description = KeywordList::Description(category.c_str(), keywordId);\n"
"	return tr(description.c_str());\n\n"
"}\n\n"
"} // namespace\n";


const char * const HEADER_TEMPLATE =

"#ifndef ${PREFIX}_KeywordListH\n"
"#define ${PREFIX}_KeywordListH\n\n"
"#include <string>\n"
"#include <typeinfo>\n\n"
"namespace ${NAMESPACE} {\n"
"/*! The class KeywordList provides conversion functionality between keyword strings\n"
"	and their respective enumeration values.\n"
"	Keyword strings are used in the Delphin project files as identifyer. In the code\n"
"	we use the enumeration values. You can obtain a keyword for a given enumeration value\n"
"	with the static function Keyword(). The corresponding enumeration value can be\n"
"	obtained with Enumeration(), as shown in the example below.\n"
"	\\code\n"
"	// to get the keyword string use\n"
"	string kw = KeywordList::Keyword(\"Assignment::type_t\", Assignment::ST_MATERIAL);\n"
"	// and to get the enumeration value\n"
"	Assignment::type_t t = KeywordList::Enumeration(\"Assignment::type_t}\", kw);\n"
"	// the string \"Assignment::type_t\" is the full enumeration type and formed from the class\n"
"	// name and the enumeration type name\n"
"	\\endcode\n"
"	Remember to pass the correct and full enumeration type as first argument to Keyword.\n"
"	\\note 	Instead of overloading the Keyword() function for different types\n"
"			in the current implementation we use a generic form in order to avoid\n"
"			adding compilation dependencies for\n"
"			all CPP files in the project that include the keyword list.\n"
"*/\n"
"class KeywordList {\n"
"public:\n"
"	/*!Returns a keyword for an enum value t of type enumtype. */\n"
"	static const char * Keyword(const char * const enumtype, int t);\n\n"
"	/*! Returns a description for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		If no descrption is given, the keyword itself is returned.\n"
"		\\param enumtype 		The full enumeration type including the class name.\n"
"		\\param t 				The enumeration type cast in an int.\n"
"		\\param no_description	The optional argument is set to true, if there was no description\n"
"		for this keyword, otherwise to false.\n"
"	*/\n"
"	static const char * Description(const char * const enumtype, int t, bool * no_description = NULL);\n\n"
"	/*! Returns a default unit for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		Returns an empty string if no default unit was specified.\n"
"		\\param enumtype The full enumeration type including the class name.\n"
"		\\param t 		The enumeration type cast in an int.\n"
"	*/\n"
"	static const char * Unit(const char * const enumtype, int t);\n\n"
"	/*! Returns a color string for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		Returns the color code for 'white' if no color value was specified.\n"
"		\\param enumtype The full enumeration type including the class name.\n"
"		\\param t 		The enumeration type cast in an int.\n"
"	*/\n"
"	static const char * Color(const char * const enumtype, int t);\n\n"
"	/*! Returns a default value for an enum value t of type enumtype.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		Returns an nan if no default value was specified.\n"
"		\\param enumtype The full enumeration type including the class name.\n"
"		\\param t 		The enumeration type cast in an int.\n"
"	*/\n"
"	static double DefaultValue(const char * const enumtype, int t);\n\n"
"	/*! Returns an enumeration value for a given keyword kw of type enumtype.\n"
"		This function throws an exception if the keyword or the enumeration type is invalid or unknown.\n"
"		\\param enumtype 	The full enumeration type including the class name.\n"
"		\\param kw 			The keyword string.\n"
"		\\param deprecated 	The optional argument is set the true if the keyword kw is deprecated.\n"
"	*/\n"
"	static int Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated = NULL);\n\n\n"

"	/*!	Returns the maximum index for entries of a category in the keyword list.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		\\param enumtype 	The full enumeration type including the class name.\n"
"	*/\n"
"	static int MaxIndex(const char * const enumtype);\n\n\n"
"	/*! Returns the number of keywords in this category.\n"
"		This function throws an exception if the enumeration type is invalid or unknown.\n"
"		\\param enumtype 	The full enumeration type including the class name.\n"
"	*/\n"
"	static unsigned int Count(const char * const enumtype);\n\n"
"	/*! Checks whether a keyword exists in the enumeration of type enumtype.\n"
"		\\return Returns true if the keyword is valid, otherwise false.\n"
"	*/\n"
"	static bool KeywordExists(const char * const enumtype, const std::string & kw);\n"
"	/*! Checks whether a category of type enumtype exists.\n"
"		\\return Returns true if the category/enum type exists, otherwise false.\n"
"	*/\n"
"	static bool CategoryExists(const char * const enumtype);\n"
"};\n\n"
"} // namespace ${NAMESPACE}\n\n"
"/*!\n"
"	\\file ${PREFIX}_KeywordList.h\n"
"	\\brief Contains the declaration of class KeywordList.\n"
"*/\n\n"
"#endif // ${PREFIX}_KeywordListH\n";


const char * const CPP_HEADER_TEMPLATE =

"#include \"${PREFIX}_KeywordList.h\"\n\n"
"#include <map>\n"
"#include <limits>\n"
"#include <iostream>\n\n"
"#include <IBK_FormatString.h>\n"
"#include <IBK_Exception.h>\n\n\n"
"namespace ${NAMESPACE} {\n";

const char * const CPP_FOOTER =

"	const char * KeywordList::Keyword(const char * const enumtype, int t) {\n"
"		const char * const kw = theKeyword(enum2index(enumtype), t);\n"
"		if (std::string(kw) == INVALID_KEYWORD_INDEX_STRING) {\n"
"			throw IBK::Exception(IBK::FormatString(\"Cannot determine keyword for enumeration type '%1' and index '%2'.\")\n"
"				.arg(enumtype).arg(t), \"[KeywordList::Keyword]\");\n"
"		}\n"
"		return kw;\n"
"	}\n"
"\n"
"	bool KeywordList::KeywordExists(const char * const enumtype, const std::string & kw) {\n"
"		int typenum = enum2index(enumtype);\n"
"		int i = 0;\n"
"		int maxIndexInCategory = MaxIndex( enumtype ); \n"
"		for ( ; i <= maxIndexInCategory; ++i ) {\n"
"			std::string keywords = allKeywords(typenum, i);\n"
"			if (keywords == INVALID_KEYWORD_INDEX_STRING)\n"
"				continue;\n"
"			std::stringstream strm(keywords);\n"
"			int j = 0;\n"
"			std::string kws;\n"
"			while (strm >> kws) {\n"
"				if (kws == kw) {\n"
"					return true; // keyword exists\n"
"				}\n"
"				++j;\n"
"			}\n"
"		}\n"
"		return false; // nothing found keyword doesn't exist.\n"
"	}\n"
"\n"
"	int KeywordList::Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated) {\n"
"		int typenum = enum2index(enumtype);\n"
"		int i = 0;\n"
"		int maxIndexInCategory = MaxIndex( enumtype ); \n"
"		for ( ; i <= maxIndexInCategory; ++i ) {\n"
"			std::string keywords = allKeywords(typenum, i);\n"
"			if (keywords == INVALID_KEYWORD_INDEX_STRING)\n"
"				continue;\n"
"			std::stringstream strm(keywords);\n"
"			int j = 0;\n"
"			std::string kws;\n"
"			while (strm >> kws) {\n"
"				if (kws == kw) {\n"
"					if (deprecated != NULL) {\n"
"						*deprecated = (j != 0);\n"
"					}\n"
"					return i;\n"
"				}\n"
"				++j;\n"
"			}\n"
"		} // for ( ; i < maxIndexInCategory; ++i ) {\n"
"		throw IBK::Exception(IBK::FormatString(\"Cannot determine enumeration value for \"\n"
"			\"enumeration type '%1' and keyword '%2'.\")\n"
"			.arg(enumtype).arg(kw), \"[KeywordList::Enumeration]\");\n"
"	}\n\n"
"	bool KeywordList::CategoryExists(const char * const enumtype) {\n"
"		return enum2index(enumtype) != -1;\n"
"	}\n"
"\n";

// ******* MAIN ********


int main(int argc, char *argv[]) {
	std::cout << "-----------------------------------------------------------------------" << std::endl;
	std::cout << "KeywordListCreator, copyright 2011 by Andreas Nicolai, Stefan Vogelsang" << std::endl;
	std::cout << "Extracting keywords from header files..." << std::endl;


	if (argc < 4 || argc > 6) {
		std::cerr << "Arguments received(" << argc << ")" << std::endl;
		for (int i=0; i<argc; ++i)
			std::cerr << "  " << argv[i] << std::endl;
		std::cerr << std::endl;

		std::cerr << "SYNTAX:  keywordlistcreator <namespace> <path/to/src> <generateQtSrc> <prefix>" << std::endl << std::endl;
		std::cerr << "         <namespace> is either DELPHIN or MM (used also to compose file names)." << std::endl;
		std::cerr << "         <path/to/<lib>/src> is a '+' separated list of input directories, for example\n"
					"         \"/home/me/model/src+/home/me/model/src/submodels\"\n"
					"         Keyword list is written to first directory in list." << std::endl;
		std::cerr << "         <generateQtSrc> is 1 when Qt source should be generated, 0 otherwise." << std::endl;
		std::cerr << "         <prefix> is the file prefix <prefix>_KeywordList.*, <namespace> is used if prefix\n"
					"         is not given." << std::endl;
		return EXIT_FAILURE;
	}

	// first argument is namespace
	NAMESPACE = argv[1];
	// second argument is encoded list of input directories
	std::string src_dir = argv[2];
	std::replace(src_dir.begin(), src_dir.end(), '\\', '/');

	std::vector< std::string > inputDirectories;
	std::string part;

	// extract ":" separated folders
	size_t pos, lastPos;
	while ( (pos = src_dir.find("+")) != std::string::npos ) {
		lastPos = 0;
		part = src_dir.substr( lastPos, (pos-lastPos) );
		if (part.empty())
			continue; // skip empty parts (i.e. src_dir starts with a ;)
		if (part[part.size()-1] == '/')
			part = part.substr(0, part.size()-1);
		lastPos = pos+1;
		inputDirectories.push_back(part);
		src_dir = src_dir.substr( lastPos );
	}

	// add single or last directory
	if (src_dir.find_first_not_of(" \t") != std::string::npos)
		inputDirectories.push_back(src_dir);

	if (inputDirectories.empty()) {
		inputDirectories.push_back(".");
	}

	// now all source dir are stored in the vector of src directories

	// third argument is path to tr() version
	bool isQtOutputRequired = false;
	std::string isOt = argv[3];
	if ( isOt == "1")
		isQtOutputRequired = true;

	// optional fourth argument is file prefix
	std::string prefix = NAMESPACE;
	if (argc > 4)
		prefix = argv[4];


	CPP_HEADER = CPP_HEADER_TEMPLATE;
	HEADER = HEADER_TEMPLATE;

	QT_CPP_HEADER = QT_CPP_HEADER_TEMPLATE;
	QT_CPP_FOOTER = QT_CPP_FOOTER_TEMPLATE;
	QT_HEADER = QT_HEADER_TEMPLATE;

	// replace two occurrances of namespace placeholder
	CPP_HEADER.replace(CPP_HEADER.find("${NAMESPACE}"),12, NAMESPACE);
	CPP_HEADER.replace(CPP_HEADER.find("${PREFIX}"),9, prefix);

	// replace 6 namespace placeholder
	HEADER.replace(HEADER.find("${NAMESPACE}"),12, NAMESPACE);
	HEADER.replace(HEADER.find("${NAMESPACE}"),12, NAMESPACE);
	HEADER.replace(HEADER.find("${PREFIX}"),9, prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, prefix);


	// replace two occurrances of namespace placeholder
	QT_HEADER.replace(QT_HEADER.find("${NAMESPACE}"),12, NAMESPACE);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, prefix);
	QT_CPP_HEADER.replace(QT_CPP_HEADER.find("${NAMESPACE}"),12, NAMESPACE);
	QT_CPP_HEADER.replace(QT_CPP_HEADER.find("${PREFIX}"),9, prefix);




	// write qt header
	if ( isQtOutputRequired ) {
		std::string fnameQt = inputDirectories[0] + "/../srcTranslations/" + prefix +"_KeywordListQt.h";
		if (!file_exists(fnameQt))
			generate_header_code( fnameQt, isQtOutputRequired );
	}

	// write header
	std::string fname = inputDirectories[0] + "/" + prefix +"_KeywordList.h";
	if (!file_exists(fname))
		generate_header_code( fname, false );

	std::vector<Keyword> keywordlist;

	// parse all directories
	unsigned int lastCount = 0;
	for (std::vector< std::string >::const_iterator it = inputDirectories.begin(),
		end = inputDirectories.end();
		it != end;
		++it)
	{
		// get list of header files
		std::vector< std::string> hfiles;
		if (!listHeaders(*it, hfiles))
			return EXIT_FAILURE;

		try {
			// parse files

			std::cout << "Parsing " << hfiles.size() << " header files" << std::endl;
			if (!parse_headers((*it), hfiles, keywordlist ))
				return EXIT_FAILURE;
			std::cout << keywordlist.size() - lastCount << " keywords in " << *it << std::endl;
			lastCount = keywordlist.size();
		}
		catch (std::exception & ex) {
			std::cerr << ex.what();
			return EXIT_FAILURE;
		}

	} // for all directories in list

	try {
		// generate code
		std::string fname = inputDirectories[0] + "/" + prefix +"_KeywordList.cpp";
		if (!generate_keywordlist_code(fname, keywordlist))
			return EXIT_FAILURE;
		// generate qt code
		if ( isQtOutputRequired ){
			std::string fnameQt = inputDirectories[0] + "/../srcTranslations/" + prefix +"_KeywordListQt.cpp";
			if (!generate_keywordlist_qt(fnameQt, keywordlist))
				return EXIT_FAILURE;
		}
		std::cout << keywordlist.size() << " keywords extracted!" << std::endl;
		std::cout << "-----------------------------------------------------------------------" << std::endl;
	}
	catch (std::exception & ex) {
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// ********* Header File Parser **************
bool parse_headers(const std::string& src_dir, const std::vector<std::string>& hfiles, std::vector<Keyword>& keywordlist) {

	//unsigned int lastnum;
	for (unsigned int i=0; i<hfiles.size(); ++i) {

		std::string fname = src_dir + '/' + hfiles[i];

		//lastnum = static_cast<unsigned int>(keywordlist.size());
		std::ifstream in(fname.c_str());
		if (!in) {
			std::cerr << "Error opening file '" << fname << "'!"<< std::endl;
			std::cerr << "For hfile '" << hfiles[i] << "'!"<< std::endl;
			return false;
		}

		//        cout << left << setw(30) << hfiles[i] << endl;    // output current file
		std::string line, classname;
		std::size_t pos;

		// Loop until end of file is reached
		while (in){

			// seek forward until the starting sequence is found
			while (getline(in, line) && line.find("***KEYWORDLIST-START***")==std::string::npos) {

				pos = line.find("class");
				if (pos != std::string::npos && (pos+6<line.size()) ) {
					classname = line.substr(pos+6);
					trim(classname);
					trimLastBracket(classname);
					if (classname.empty()) {
						in >> classname;
						continue;
					}
					pos = classname.find_first_of(" \t\n");
					if (pos==std::string::npos) continue;
					classname.erase(pos);
				}

			}

			// trim : from classname
			size_t f = classname.find_first_not_of(":");
			size_t l = classname.find_last_not_of(":");
			if (f!=std::string::npos && l!=std::string::npos)
				classname = classname.substr(f,l-f+1);

			// no starting sequence found, skip the rest
			if (!in) {
				//                if (keywordlist.size() - lastnum==0)
				//                    cout << " - nix" << endl;
				continue;
			}

			int keycount=0;
			std::string category;

			// necessary for resolving CONSTANT_RESOLVE lines
			bool	replacementCompileTimeConstantFound = false;
			std::map< std::string, std::string > constantsMappings;
			std::map< std::string, int > mapCompileTimeConstantToValue;

			// loop until ending sequence
			while (getline(in, line) && line.find("***KEYWORDLIST-END***")==std::string::npos) {

				if (line.empty())
					continue;

				// found new enumeration?
				pos=line.find("***CONSTANT_RESOLVE ");
				if (pos!=std::string::npos) {

					replacementCompileTimeConstantFound = true;

					// cut first part
					std::string partLine = line.substr(pos+20, std::string::npos);

					// trim both ends of spaces
					trim(partLine);

					// split line at space
					size_t posSpace = partLine.find_first_of(" \t");

					// devide into parts
					std::string constant = partLine.substr(0, posSpace );
					std::string path = partLine.substr(posSpace, std::string::npos);

					// trim again
					trim(constant);
					trim(path);

					// assign
					constantsMappings[constant] = path;
					continue;

				}

				// now that we have all the repleacement string we need to resolve them
				if (replacementCompileTimeConstantFound){

					for (
							std::map< std::string, std::string >::const_iterator it = constantsMappings.begin(),
							end = constantsMappings.end();
							it != end;
							++it
						)
					{

						// open file for replacements

						std::string relativePath =  src_dir + "/" + it->second;
						std::cout << "Sourcing constant Keywords from file: " << relativePath << std::endl;
						std::ifstream inIncludeFile( relativePath.c_str() );
						if (!inIncludeFile) {
							std::cerr << "Error opening file '" << it->second << "',"<< std::endl;
							std::cerr << "referenced in ***CONSTANT_RESOLVE line from file '"<< fname << "'." << std::endl;
							throw std::runtime_error( "Error processing CONSTANT_RESOLVE directive." );
						}

						// now we have to search for a class pattern, and a constant value in an enum
						// possible values NAMESPACE::CLASS::ENUM_VALUE; CLASS::ENUM_VALUE

						size_t lastQuad = it->first.find_last_of(':');
						size_t firstQuad = it->first.find_first_of(':');

						if ( (lastQuad == std::string::npos) || (firstQuad == std::string::npos) ){
//							std::cerr << "Invald target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE or CLASS::ENUM_VALUE expected." << std::endl;
							throw std::runtime_error( "Invalid target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE or CLASS::ENUM_VALUE expected." );
						}

						std::string searchEnumValue = it->first.substr(lastQuad+1, std::string::npos);
						std::string searchClass;
						if ( firstQuad == lastQuad-1 ){
							// CLASS::ENUM_VALUE
							searchClass = it->first.substr( 0, firstQuad );
						} else {
							// NAMESPACE::CLASS::ENUM_VALUE
							searchClass = it->first.substr( 0, lastQuad-1 );
							// NAMESPACE::CLASS
							pos = searchClass.find_last_of(':');
							if (pos != std::string::npos){
								searchClass = searchClass.substr( pos+1, std::string::npos );
							} else {
//								std::cerr << "Invald target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE expected." << std::endl;
								throw std::runtime_error( "Invald target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE expected." );
							}
						}
						std::string includeLine, includeClassName;

//						// search replacement
//						while(inIncludeFile){

							// seek forward until a class is found
							while ( getline(inIncludeFile, includeLine) ) {

								pos = includeLine.find("class");
								if (pos != std::string::npos && (pos+6<includeLine.size()) ) {
									includeClassName = includeLine.substr(pos+6);
									trim(includeClassName);
									trimLastBracket(includeClassName);
									if (includeClassName.empty()) {
										inIncludeFile >> includeClassName;
										continue;
									}
									pos = includeClassName.find_first_of(" \t\n");
									if (pos==std::string::npos) continue;
									includeClassName.erase(pos);
								}

								// found it so we go over to search the enum value
								if ( searchClass == includeClassName ){

									// now seek forward to find enum, just one overloaded enum value is allowed at a time
									int counterValue = 0;
									while ( getline(inIncludeFile, includeLine) )
									{
										// check if we left the class, if we are done we can leave
										pos=includeLine.find("***KEYWORDLIST-END***");
										if (pos != std::string::npos)
											break;

										pos=includeLine.find("enum ");
										if (pos != std::string::npos){

											counterValue = 0;
											while ( getline(inIncludeFile, includeLine) ) {


												// check if we finaly have found the constant we looking for
												size_t posEnumValue = includeLine.find( searchEnumValue );
												if ( posEnumValue != std::string::npos ){

													// store index in map
													mapCompileTimeConstantToValue[ it->first ] = counterValue;
													break;
												}


												// calc start value
												// if a line contains a =
												// search for a = to fix enumeration
												size_t posEqual = includeLine.find("=");
												size_t pos = includeLine.find("Keyword:");

												// we found an enumeration now we have to extract it
												if (posEqual<pos){

													unsigned int base;
													int resultI = 0;
													unsigned int size;

													// get substring containing only the number
													std::string number;
													size_t posNumberEnd = includeLine.find(",");
													if ( posNumberEnd != std::string::npos ) {
														number = includeLine.substr( posEqual+1, posNumberEnd-posEqual-1 );
													} else {
														size_t posNumberEnd = includeLine.find("/");
														number = includeLine.substr( posEqual+1, posNumberEnd-posEqual-1 );
													}

													// strip all spaces
													number = stripSpaces(number);

													// hex or no hex
													posEqual = number.find("0x");
													if ( posEqual == std::string::npos ){

														// unsigned int or int found
														posEqual = number.find("-");
														base = 1;
														if ( posEqual == std::string::npos ){

															// unsigned int found
															// copy number in reverse
															// convert hex string to unsigned int
															size = number.size();
															while (size--){

																// is character a number
																if ( number[size] < 58 && number[size] > 47 ){
																	resultI += (number[size]-48) * base;
																	base *= 10;
																} else {
																	throw std::runtime_error("Invalid decimal format in line '"+includeLine+"'.");
																}

															} // while (size--){


														} else {

															// int found
															throw std::runtime_error("Invalid int format not supported yet.");

														}

													} else {

														// copy number in reverse
														// hexadecimal number found
														// convert hex string to unsigned int

														// strip hex preface
														number = number.substr( posEqual+2 );
														// strip spaces
														size = number.size();
														base = 1;

														while (size--){

															// is character a number
															if ( number[size] < 58 && number[size] > 47 ){

																resultI += (number[size]-48) * base;
																base *= 16;

															} else {

																// upper case ascii hex
																if ( number[size] < 71 && number[size] > 64 ){

																	resultI += (number[size]-55) * base;
																	base *= 16;

																} else {

																	// lower case ascii hex
																	if ( number[size] < 103 && number[size] > 96 ){

																		resultI += (number[size]-87) * base;
																		base *= 16;

																	} else {

																		// error during parsing
																		/// \todo print some error here
																		throw std::runtime_error("Invalid hexadecimal format.");

																	} // else if (number[size]){

																} // else if (number[size]){

															}  // else if ( number[size] < 58 && number[size] > 47 ){

														} // while (size--) {

													} // else if ( posEqual == std::string::npos ){

													counterValue = resultI;

												} else { // if (posEqual<pos){

													// just increment line count
													counterValue++;

												} // if (posEqual<pos){


											} // while ( getline(inIncludeFile, includeLine) )

										} // if (pos != std::string::npos){


									} // while ( getline(inIncludeFile, includeLine) )

									break;

								} // if ( searchClass == includeClassName ){

							} // // seek forward until a class is found



//						} // while(inIncludeFile){

					} // for all mappings

					// reset all resolve mappings
					constantsMappings.clear();
					replacementCompileTimeConstantFound = false;

				} // if (replacementCompileTimeConstantFound){


				/**********************
					*
					* start new enum field
					*
				***********************/



				// found new enumeration?
				pos=line.find("enum ");
				if (pos!=std::string::npos) {
					category=line.substr(pos+5, std::string::npos);
					pos = category.find("{");
					if (pos==std::string::npos) {
						std::cerr << " - error in enumeration '" << line << "'. '{' is missing!" << std::endl;
						return false;
					}
					category.erase( pos );
					trim(category);
					keycount=0;

					continue;
				}

				// we require the string 'Keyword:', this string cannot be used in the description or
				// any other place in the code
				pos=line.find("Keyword:");
				if (pos != std::string::npos){

					Keyword kw;
					try {
						kw = parse_keyword_line( line, mapCompileTimeConstantToValue );
					}
					catch (std::exception & ex) {
						std::cerr << ex.what() << std::endl;
						throw std::runtime_error("Error in file '"+fname+"'");
					}
					if ( kw.index==-1 )
						continue;  // no keyword found

					// set index according to user wishes and store it to the keycounter
					// increment the
					if ( kw.index==-2 )
						kw.index = keycount++;
					else
						keycount = kw.index + 1;

					if ( classname.empty() )
						kw.category = category;
					else
						kw.category = (classname + "::" + category);
					keywordlist.push_back(kw);

					for ( std::vector< Keyword >::iterator it = keywordlist.begin(); it != keywordlist.end(); it++) {
						if ( (*it).category == kw.category ) {
							(*it).maxIndexInCategory = kw.index;
						};
					}

				} // if we found a new keyword

			} // while (getline(in, line) && line.find("***KEYWORDLIST-END***")==std::string::npos) {


		}  // over all loop
	}
	return true;
}

// ********* Code Generation Functions **************

bool generate_header_code(const std::string& fname, bool qtHeader){

	std::ofstream hpp(fname.c_str());
	if (!hpp) {
		std::cerr << "Cannot create '" + fname + "'." << std::endl;
		return false;
	}
	// write file header
	if (qtHeader){
		hpp << QT_HEADER;
	} else {
		hpp << HEADER;
	}
	return true;

}

bool generate_keywordlist_qt( const std::string& fname, const std::vector<Keyword>& keywordlist ) {

	// first we generate the header file
	std::ofstream cpp(fname.c_str());
	if (!cpp) {
		std::cerr << "Cannot create '" + fname + "'." << std::endl;
		return false;
	}
	// write file header
	cpp << QT_CPP_HEADER;

	// for all description place tr macros here
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		std::string desc = keywordlist[i].description;
		if (desc.empty()) {
			cpp << "	tr(\"" << keywordlist[i].keyword << "\");\n";
		}
		else {
			cpp << "	tr(\"" << desc << "\");\n";
		}
	}

	// write footer
	cpp << QT_CPP_FOOTER;

	return true;
}

bool generate_keywordlist_code( const std::string& fname, const std::vector<Keyword>& keywordlist ) {
	// first we generate the header file
	std::ofstream cpp(fname.c_str());
	if (!cpp) {
		std::cerr << "Cannot create '" + fname + "'." << std::endl;
		return false;
	}
	// write file header
	cpp << CPP_HEADER;
	cpp <<
		"	/*! Holds the string to print as error when an invalid keyword is encountered. */\n"
		"	const char * const INVALID_KEYWORD_INDEX_STRING = \"KEYWORD_ERROR_STRING: Invalid type index\";\n\n";

	// get category list
	std::string lastCategory;
	std::vector<std::string> catnames;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new declaration
		if (keywordlist[i].category != lastCategory) {
			lastCategory = keywordlist[i].category;
			catnames.push_back(lastCategory);
		}
	}
	if (catnames.empty()) {
		std::cerr << "No categories extracted!" << std::endl;
		return false;
	}
	// write category array
	cpp <<
		"	/*! Holds a list of all enum types/categories. */\n"
		"	const char * const ENUM_TYPES["<< catnames.size() << "] = {\n";
	for (unsigned int i=0; i<catnames.size()-1; ++i) {
		cpp << "\t\t\"" + catnames[i] + "\",\n";
	}
	cpp << "\t\t\"" + catnames.back() + "\"\n";
	cpp << "\t};\n\n";

	// write middle part
	cpp <<
		"	/*! Converts a category string to respective enumeration value. */ \n"
		"	int enum2index(const std::string & enumtype) {\n"
		"		for (int i=0; i<"<< catnames.size() << "; ++i) {\n"
		"			if (enumtype == ENUM_TYPES[i]) return i;\n"
		"		}\n"
		"		//std::cerr << \"Unknown enumeration type '\" << enumtype<< \"'.\" << std::endl;\n"
		"		return -1;\n"
		"	}\n"
		"	\n\n";

	// *** write theKeyword() function ***
	cpp <<
		"	/*! Returns a keyword string for a given category (typenum) and type number t. */ \n"
		"	const char * theKeyword(int typenum, int t) {\n"
		"		switch (typenum) {\n";

	lastCategory.clear();
	int switchIndex = 0;
//	int localSwitchIndex = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new switch clause
		if (keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break; \n";
				++switchIndex;
			}
			lastCategory = keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) { \n";
		}
		std::string kw = keywordlist[i].keyword;
		size_t space = kw.find_first_of(" \t");
		if (space != std::string::npos)
			kw = kw.substr(0,space);
		cpp << "				case " << keywordlist[i].index << " : return \"" << kw << "\";\n";
	}
	// close last switch clause and close switch statement
	cpp << "			} break; \n";
	cpp << "		} // switch\n";
	cpp <<	"		return INVALID_KEYWORD_INDEX_STRING;\n"
			"	}\n\n";

	// *** write allKeywords() function ***
	cpp <<
		"	/*! Returns all keywords including deprecated for a given category (typenum) and type number (t). */ \n"
		"	const char * allKeywords(int typenum, int t) {\n"
		"		switch (typenum) {\n";

	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new switch clause
		if (keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break; \n";
				++switchIndex;
			}
			lastCategory = keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) { \n";
		}
		cpp << "				case " << keywordlist[i].index << " : return \"" << keywordlist[i].keyword << "\";\n";
	}
	// close last switch clause and close switch statement
	cpp << "			} break; \n";
	cpp << "		} // switch\n";
	cpp <<	"		return INVALID_KEYWORD_INDEX_STRING;\n"
			"	}\n\n";

	// *** write description function ***
	cpp <<	"	const char * KeywordList::Description(const char * const enumtype, int t, bool * no_description) {\n"
			"		if (no_description != NULL)\n"
			"			*no_description = false; // we are optimistic\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new switch clause
		if (keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break; \n";
				++switchIndex;
			}
			lastCategory = keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) { \n";
		}

		std::string desc = keywordlist[i].description;
		if (desc.empty()) {
			cpp << "				case " << keywordlist[i].index << " : if (no_description != NULL) *no_description = true; return \"" << keywordlist[i].keyword << "\";\n";
		}
		else {
			cpp << "				case " << keywordlist[i].index << " : return \"" << desc << "\";\n";
		}
	}

	cpp <<	"			} break; \n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine description for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::Description]\");\n"
			"	}\n\n";


	// *** write unit() function ***
	cpp <<	"	const char * KeywordList::Unit(const char * const enumtype, int t) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new switch clause
		if (keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break; \n";
				++switchIndex;
			}
			lastCategory = keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) { \n";
		}
		cpp << "				case " << keywordlist[i].index << " : return \"" << keywordlist[i].unit << "\";\n";
	}

	cpp <<	"			} break; \n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine default unit for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::Unit]\");\n"
			"	}\n\n";


	// *** write color() function ***
	cpp <<	"	const char * KeywordList::Color(const char * const enumtype, int t) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new switch clause
		if (keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break; \n";
				++switchIndex;
			}
			lastCategory = keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) { \n";
		}
		cpp << "				case " << keywordlist[i].index << " : return \"" << keywordlist[i].color << "\";\n";
	}

	cpp <<	"			} break; \n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine color for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::Color]\");\n"
			"	}\n\n";


	// *** write defaulValue() function ***
	cpp <<	"	double KeywordList::DefaultValue(const char * const enumtype, int t) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {
		// new category = new switch clause
		if (keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break; \n";
				++switchIndex;
			}
			lastCategory = keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) { \n";
		}
		if ( keywordlist[i].defaultValue == keywordlist[i].defaultValue ) {
			cpp << "				case " << keywordlist[i].index << " : return " << keywordlist[i].defaultValue << ";\n";
		} else {
			cpp << "				case " << keywordlist[i].index << " : return std::numeric_limits<double>::quiet_NaN();\n";
		}
	}

	cpp <<	"			} break; \n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine default value for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::DefaultValue]\");\n"
			"	}\n\n";


	// *** write Count() function ***
	cpp <<	"	// number of entries in a keyword list \n";
	cpp <<	"	unsigned int KeywordList::Count(const char * const enumtype) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	unsigned int categoryCount = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {

		// new category = new entry in switch
		if (keywordlist[i].category != lastCategory) {

			// if last category wasn't empty, close last case clause
			if (!lastCategory.empty()) {
				cpp << categoryCount << ";\n";
				++switchIndex;
				categoryCount = 0;
			}

			lastCategory = keywordlist[i].category;
			// open new case clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" : return ";
		}
		++categoryCount;
	}

	cpp <<	categoryCount << ";\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Invalid enumeration type '%1'.\")\n"
			"			.arg(enumtype), \"[KeywordList::Count]\");\n"
			"	}\n\n";

	cpp <<	"	// max index for entries sharing a category in a keyword list \n";
	cpp <<	"	int KeywordList::MaxIndex(const char * const enumtype) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	categoryCount = 0;
	for (unsigned int i=0; i<keywordlist.size(); ++i) {

		// new category = new entry in switch
		if (keywordlist[i].category != lastCategory) {

			// if last category wasn't empty, close last case clause
			if (!lastCategory.empty()) {
				cpp << keywordlist[i-1].maxIndexInCategory << ";\n";
				++switchIndex;
				categoryCount = 0;
			}

			lastCategory = keywordlist[i].category;
			// open new case clause
			cpp << "			// "<< lastCategory <<" \n";
			cpp << "			case "<< switchIndex <<" : return ";
		}
		++categoryCount;
	}

	cpp << keywordlist[keywordlist.size()-1].maxIndexInCategory << ";\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Invalid enumeration type '%1'.\")\n"
			"			.arg(enumtype), \"[KeywordList::MaxIndex]\");\n"
			"	}\n\n";


	// write file footer
	cpp << CPP_FOOTER;
	cpp << "} // namespace " << NAMESPACE << std::endl;
	return true;
}

// ********* Utility functions **************

bool file_exists(const std::string& dirname) {
#ifdef WIN32
	// code replaced by WinAPI function because lower code doesn't work correctly in MinGW
	DWORD dwAttrib = GetFileAttributes(dirname.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		   !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

	// implementation for MS VC
//	struct _stat buf;
//	int res = _stat(dirname.c_str(), &buf);
//	if (res == -1) return false;
//	// check if it is a directory
//	if (buf.st_mode & _S_IFDIR) return true;
//	else						return true;
#else
	struct stat buf;
	int res = stat(dirname.c_str(), &buf);
	if (res == -1) {
		switch (errno) {
			case EACCES : std::cerr << "Cannot access file/directory '"+dirname+"'!\n"; break;
		}
		return false;
	}
	// check if it is a directory
	if (S_ISDIR(buf.st_mode))	return true;
	else						return true;
#endif
}


bool listHeaders(const std::string & dir, std::vector<std::string> & files) {
#ifdef WIN32
	WIN32_FIND_DATA fd;
	std::string tmpdir = dir + "/*";
	HANDLE fh = FindFirstFile((LPCSTR)tmpdir.c_str(), &fd);
	if (fh == INVALID_HANDLE_VALUE) {
		std::cerr << "Cannot open/read directory: '"<< dir<< "'" << std::endl;
		return false;
	}
	do {
		std::string f = fd.cFileName;
		if (f.size() > 2 && f.find(".h") == f.size()-2)
			files.push_back(f);
	}
	while(FindNextFile(fh, &fd));
	FindClose(fh);
#else
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dir.c_str());
	if (dp != NULL) {
		while ( (ep = readdir (dp)) ) {
			std::string f = ep->d_name;
			if (f.size() > 2 && f.find(".h") == f.size()-2)
				files.push_back(f);
		}
		closedir(dp);
	}
	else {
		std::cerr << "Cannot open/read directory: '"<< dir<< "'" << std::endl;
	}
#endif // WIN32
	std::sort(files.begin(), files.end());
	return true;
}

void trim(std::string& str) {
	size_t f = str.find_first_not_of(" \t\r\n");
	size_t l = str.find_last_not_of(" \t\r\n");
	if (f!=std::string::npos && l!=std::string::npos)
		str = str.substr(f,l-f+1);
}

void trimLastBracket(std::string& str) {
	size_t l = str.find_last_not_of("{");
	if (l!=std::string::npos)
		str = str.substr(0,l+1);
}


std::string stripSpaces(const std::string & input) {
	std::string tmp;

	for (unsigned int inputStringIndex = 0, stringSize = input.size();
		inputStringIndex < stringSize; inputStringIndex++ )
	{

		if (input[inputStringIndex] != ' ' && input[inputStringIndex] != '\t'){
			tmp.push_back(input[inputStringIndex]);
		}

	}
	return tmp;
}


/*!
	return type kw.index
	0...32000 setup index
	-1 error
	-2 enumeration without set index
*/
Keyword parse_keyword_line(const std::string& line, std::map< std::string, int > mapReplacementsToInt ) {

	Keyword kw;
	kw.index = -2;
	unsigned int base;
	int resultI = 0;
	unsigned int size;


	// search for a = to fix enumeration
	size_t posEqual = line.find("=");
	size_t pos = line.find("Keyword:");

	// we found an enumeration now we have to extract it
	if (posEqual<pos){

		// get substring containing only the number
		std::string number;
		size_t posNumberEnd = line.find(",");
		if ( posNumberEnd != std::string::npos ) {
			number = line.substr( posEqual+1, posNumberEnd-posEqual-1 );
		} else {
			size_t posNumberEnd = line.find("/");
			number = line.substr( posEqual+1, posNumberEnd-posEqual-1 );
		}

		// strip all spaces
		number = stripSpaces(number);

		// check if number needs to be replaced by integer representing value
		for (
			 std::map< std::string, int >::const_iterator it = mapReplacementsToInt.begin(),
			 end = mapReplacementsToInt.end();
			 it != end;
			 ++it
			)
		{

			// check if we can find th complete replacement code
			size_t posReplacements = number.find( it->first );
			if ( posReplacements != std::string::npos ){
				// found it so we assign the kw index and go
				kw.index = it->second;
				goto noParsingNeeded;
			}

		}


		// hex or no hex
		posEqual = number.find("0x");
		if ( posEqual == std::string::npos ){

			// unsigned int or int found
			posEqual = number.find("-");
			base = 1;
			if ( posEqual == std::string::npos ){

				// unsigned int found
				// copy number in reverse
				// convert hex string to unsigned int
				size = number.size();
				while (size--){

					// is character a number
					if ( number[size] < 58 && number[size] > 47 ){
						resultI += (number[size]-48) * base;
						base *= 10;
					} else {
						throw std::runtime_error("Invalid decimal format in line '"+line+"'.");
					}

				} // while (size--){


			} else {

				// int found
				throw std::runtime_error("Invalid int format not supported yet.");

			}

		} else {

			// copy number in reverse
			// hexadecimal number found
			// convert hex string to unsigned int

			// strip hex preface
			number = number.substr( posEqual+2 );
			// strip spaces
			size = number.size();
			base = 1;

			while (size--){

				// is character a number
				if ( number[size] < 58 && number[size] > 47 ){

					resultI += (number[size]-48) * base;
					base *= 16;

				} else {

					// upper case ascii hex
					if ( number[size] < 71 && number[size] > 64 ){

						resultI += (number[size]-55) * base;
						base *= 16;

					} else {

						// lower case ascii hex
						if ( number[size] < 103 && number[size] > 96 ){

							resultI += (number[size]-87) * base;
							base *= 16;

						} else {

							// error during parsing
							/// \todo print some error here
							throw std::runtime_error("Invalid hexadecimal format.");

						} // else if (number[size]){

					} // else if (number[size]){

				}  // else if ( number[size] < 58 && number[size] > 47 ){

			} // while (size--) {

		} // else if ( posEqual == std::string::npos ){

		kw.index = resultI;

	} // if (posEqual!=std::string::npos){


	// this is ugly I know but we can shortcut a lot of code here
	noParsingNeeded:

	if (pos==std::string::npos){
		kw.index = -1;
		return kw; // index==-1  -> no Keyword extracted
	}

	std::string remaining_line = line.substr(pos+9, std::string::npos);
	trim(remaining_line);

	if (remaining_line.empty()){
		kw.index = -1;
		return kw; // index==-1  -> no Keyword extracted
	}

	// we parse the string back to front, starting with the description text
	size_t substart = remaining_line.find('\'');
	size_t subend = remaining_line.rfind('\'');
	if (substart != std::string::npos && subend != std::string::npos) {
		if (subend == substart)
			throw std::runtime_error("Found only on ' in keyword line.");
		kw.description = remaining_line.substr(substart+1, subend - substart - 1);
		trim(kw.description);
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}


	// do we have a default value?
	kw.defaultValue = std::numeric_limits<double>::quiet_NaN();
	subend = remaining_line.rfind('}');
	substart = remaining_line.rfind('{', subend);
	if (substart != std::string::npos) {
		if (subend == std::string::npos)
			throw std::runtime_error("Missing closing } in keyword line.");
		std::string defaultValue = remaining_line.substr(substart+1, subend - substart - 1);
		trim(defaultValue);
		// convert
		try {
			kw.defaultValue=string2val(defaultValue);
		} catch(...){
				throw std::runtime_error("Double conversion failed.");
		}
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}
	else {
		if (subend != std::string::npos)
			throw std::runtime_error("Found closing } without opening { in keyword line.");
	}


	// do we have a color value?
	kw.color = "#FFFFFF";
	subend = remaining_line.rfind('>');
	substart = remaining_line.rfind('<', subend);
	if (substart != std::string::npos) {
		if (subend == std::string::npos)
			throw std::runtime_error("Missing closing > in keyword line.");
		std::string colorValue = remaining_line.substr(substart+1, subend - substart - 1);
		trim(colorValue);
		kw.color = colorValue;
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}
	else {
		if (subend != std::string::npos)
			throw std::runtime_error("Found closing > without opening < in keyword line.");
	}


	// do we have a default unit?
	subend = remaining_line.rfind(']');
	substart = remaining_line.rfind('[', subend);
	if (substart != std::string::npos) {
		if (subend == std::string::npos)
			throw std::runtime_error("Missing closing ] in keyword line.");
		kw.unit = remaining_line.substr(substart+1, subend - substart - 1);
		trim(kw.unit);
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}
	else {
		if (subend != std::string::npos)
			throw std::runtime_error("Found closing ] without opening [ in keyword line.");
	}


	if (remaining_line.empty())
		throw std::runtime_error("Missing keyword before unit or description.");
	kw.keyword = remaining_line; // may contain multiple keywords

	return kw;
}

std::ostream& operator<<(std::ostream& os, const Keyword& kw) {
	return os << "   \"" << kw.category << "  " << std::setw(2) << kw.index  << "  " << kw.keyword << "\\n\"" << std::endl;
}

bool write_keyword_data(const std::string& output_file, const std::vector<Keyword>& keywordlist) {
	std::string outfile = output_file;
	replace(outfile.begin(), outfile.end(), '\\', '/');
	std::ofstream out( outfile.c_str());
	// open our template file
	std::ifstream tmp( (outfile+".in").c_str() );
	if (!tmp) {
		std::cerr << "Cannot open template file '"<< (outfile+".in") << "'!." << std::endl;
		return false;
	}
	std::string line;
	bool done = false;
	out << "// WARNING: This is an automatically generated file based on the template file\n"
		<< "//          '" << (outfile+".in") << "'\n"
		<< "//          All changes made to this file will be lost during the next build!\n\n";
	while (getline(tmp, line)) {
		if (!done && line.find("### KEYWORDDATA ###")!=std::string::npos) {
			out << "const char * const KEYWORD_DATA = \n";
			copy(keywordlist.begin(), keywordlist.end(), std::ostream_iterator<Keyword>(out));
			out << "    ;\n";
			done = true;
		}
		else {
			out << line << '\n';
		}
	}
	return out.good();
}

/*! Attempts to extract a numerical value from a string.
	\code
	double val = string2val<double>("2.5");
	\endcode
*/
double string2val(const std::string& str) {
	double val;
	if (str=="1.#QNAN")
		return std::numeric_limits<double>::quiet_NaN();
	std::stringstream strm(str);
	if (!(strm >> val))
		throw std::runtime_error( "Could not convert"+str+"into value." );
	return val;
}

