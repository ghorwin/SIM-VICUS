#include "CodeGenerator.h"

#include <sstream>
#include <iostream>
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
#include <string.h>

#endif // WIN32

#include "constants.h"

bool listHeaders(const std::string & dir, std::vector<std::string> & files);

bool write_keyword_data(const std::string& output_file, const std::vector<ClassInfo::Keyword>& keywordlist);


void CodeGenerator::handleArguments(const char * const argv[]) {
	// first argument is namespace
	m_namespace = argv[1];
	// second argument is encoded list of input directories
	std::string src_dir = argv[2];
	std::replace(src_dir.begin(), src_dir.end(), '\\', '/');

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
		m_inputDirectories.push_back(part);
		src_dir = src_dir.substr( lastPos );
	}

	// add single or last directory
	if (src_dir.find_first_not_of(" \t") != std::string::npos)
		m_inputDirectories.push_back(src_dir);

	if (m_inputDirectories.empty()) {
		m_inputDirectories.push_back(".");
	}

	// now all source dir are stored in the vector of src directories

	// third argument is path to tr() version
	m_isQtOutputRequired = false;
	std::string isOt = argv[3];
	if ( isOt == "1")
		m_isQtOutputRequired = true;

	// fourth argument is file prefix
	m_prefix = argv[4];

	// target directory for ncg files
	m_ncgOutputDir = argv[5];
}


bool CodeGenerator::parseDirectories() {

	// parse all directories
	unsigned int lastCount = 0;
	for (std::vector< std::string >::const_iterator it = m_inputDirectories.begin(),
		end = m_inputDirectories.end();
		it != end;
		++it)
	{
		// get list of header files
		std::vector< std::string> hfiles;
		if (!listHeaders(*it, hfiles)) {
			std::cerr << "Please check the supplied list of input directories!" << std::endl;
			return false; // something wrong with the input directories
		}

		try {
			// parse files

			std::cout << "Parsing " << hfiles.size() << " header files" << std::endl;
			for (const std::string & h : hfiles) {
				ClassInfo c;
				if (!c.parse(IBK::Path(*it) / h)) {
					std::cerr << "Error while parsing input files." << std::endl;
					return false;
				}
				m_classInfo.push_back(c);
				m_keywordlist.insert(m_keywordlist.end(), c.m_keywords.begin(), c.m_keywords.end());
			}
			std::cout << m_keywordlist.size() - lastCount << " keywords in " << *it << std::endl;
			lastCount = m_keywordlist.size();
		}
		catch (std::exception & ex) {
			std::cerr << ex.what();
			return false;
		}

	} // for all directories in list

}


void CodeGenerator::generateKeywordList() {
	FUNCID(CodeGenerator::generateKeywordList);

	CPP_HEADER = CPP_HEADER_TEMPLATE;
	HEADER = HEADER_TEMPLATE;

	QT_CPP_HEADER = QT_CPP_HEADER_TEMPLATE;
	QT_CPP_FOOTER = QT_CPP_FOOTER_TEMPLATE;
	QT_HEADER = QT_HEADER_TEMPLATE;

	// replace two occurrances of namespace placeholder
	CPP_HEADER.replace(CPP_HEADER.find("${NAMESPACE}"),12, m_namespace);
	CPP_HEADER.replace(CPP_HEADER.find("${PREFIX}"),9, m_prefix);

	// replace 6 namespace placeholder
	HEADER.replace(HEADER.find("${NAMESPACE}"),12, m_namespace);
	HEADER.replace(HEADER.find("${NAMESPACE}"),12, m_namespace);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);


	// replace two occurrances of namespace placeholder
	QT_HEADER.replace(QT_HEADER.find("${NAMESPACE}"),12, m_namespace);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_CPP_HEADER.replace(QT_CPP_HEADER.find("${NAMESPACE}"),12, m_namespace);
	QT_CPP_HEADER.replace(QT_CPP_HEADER.find("${PREFIX}"),9, m_prefix);

	// write qt header
	if ( m_isQtOutputRequired ) {
		IBK::Path outputPath(m_inputDirectories[0] + "/../srcTranslations");
		if (!outputPath.exists())
			throw IBK::Exception(IBK::FormatString("Output directory '%1' doesn't exist.").arg(outputPath), FUNC_ID);
		IBK::Path fnameQt = outputPath / (m_prefix +"_KeywordListQt.h");
		if (!fnameQt.exists())
			generateHeaderCode( fnameQt, true );
	}

	// write header
	IBK::Path fname = IBK::Path(m_inputDirectories[0]) / (m_prefix +"_KeywordList.h");
	if (!fname.exists())
		generateHeaderCode( fname, false );

	// now process all source directories and parse input files

	try {
		// generate code
		IBK::Path fnameCPP = IBK::Path(m_inputDirectories[0]) / (m_prefix +"_KeywordList.cpp");
		generateKeywordlistCode(fnameCPP);
		// generate qt code
		if ( m_isQtOutputRequired ){
			IBK::Path outputPath(m_inputDirectories[0] + "/../srcTranslations");
			IBK::Path fnameQt = outputPath / m_prefix +"_KeywordListQt.cpp";
			generateKeywordlistCodeQt(fnameQt);
		}
		std::cout << m_keywordlist.size() << " keywords extracted!" << std::endl;
		std::cout << "-----------------------------------------------------------------------" << std::endl;
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error generating keyword list.", FUNC_ID);
	}
	catch (std::exception & ex) {
		std::cerr << ex.what() << std::endl;
		throw IBK::Exception("Error generating keyword list.", FUNC_ID);
	}
}


void CodeGenerator::generateReadWriteCode() {

}



// ********* Private Utility functions **************

bool CodeGenerator::listHeaders(const std::string & dir, std::vector<std::string> & files) {
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


void CodeGenerator::generateHeaderCode(const IBK::Path & headerFilePath, bool qtHeader) const {
	FUNCID(CodeGenerator::generateHeaderCode);
	std::ofstream hpp(headerFilePath.c_str());
	if (!hpp)
		throw IBK::Exception(IBK::FormatString("Cannot create '%1'.").arg(headerFilePath), FUNC_ID);
	// write file header
	if (qtHeader){
		hpp << QT_HEADER;
	} else {
		hpp << HEADER;
	}
}


void CodeGenerator::generateKeywordlistCode(const IBK::Path & keywordListCpp) {

}


void CodeGenerator::generateKeywordlistCodeQt(const IBK::Path & keywordListCpp)
{

}




#if 0

// ********* Code Generation Functions **************

bool generate_header_code(const std::string& fname, bool qtHeader){

}

bool generate_keywordlist_qt( const std::string& fname, const std::vector<ClassInfo::Keyword>& keywordlist ) {

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

bool generate_keywordlist_code( const std::string& fname, const std::vector<ClassInfo::Keyword>& keywordlist ) {
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

std::ostream& operator<<(std::ostream& os, const ClassInfo::Keyword& kw) {
	return os << "   \"" << kw.category << "  " << std::setw(2) << kw.index  << "  " << kw.keyword << "\\n\"" << std::endl;
}

bool write_keyword_data(const std::string& output_file, const std::vector<ClassInfo::Keyword>& keywordlist) {
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
			copy(keywordlist.begin(), keywordlist.end(), std::ostream_iterator<ClassInfo::Keyword>(out));
			out << "    ;\n";
			done = true;
		}
		else {
			out << line << '\n';
		}
	}
	return out.good();
}
#endif
