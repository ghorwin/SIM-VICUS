#ifndef CodeGeneratorH
#define CodeGeneratorH

#include "ClassInfo.h"

/*! This main implementation class of the code generator. */
class CodeGenerator {
public:
	void handleArguments(const char * const argv[]);

	bool parseDirectories();

	void generateKeywordList();

	void generateReadWriteCode();

private:
	/*! Returns a sorted list of *.h files in the given directory. */
	static bool listHeaders(const std::string & dir, std::vector<std::string> & files);

	void generateHeaderCode(const IBK::Path & headerFilePath, bool qtHeader) const;
	void generateKeywordlistCode(const IBK::Path & keywordListCpp);
	void generateKeywordlistCodeQt(const IBK::Path & keywordListCpp);

	std::vector<ClassInfo::Keyword> m_keywordlist;
	std::vector<ClassInfo>			m_classInfo;
	std::vector< std::string >		m_inputDirectories;

	/*! The namespace to put the KeywordList class and the ncg-class declarations into. */
	std::string						m_namespace;
	/*! If true, Qt keywordlist output is generated. */
	bool							m_isQtOutputRequired;
	/*! The file prefix for <prefix>_KeywordList.cpp. */
	std::string						m_prefix;
	/*! The file path to the directory where ncg files are being generated.
		Can be a relative path, in which case this is handled as a subdirectory to the
		directory where the original header file is in.
	*/
	IBK::Path						m_ncgOutputDir;

	std::string						CPP_HEADER;
	std::string						HEADER;

	std::string						QT_HEADER;
	std::string						QT_CPP_HEADER;
	std::string						QT_CPP_FOOTER;

};

#endif // CodeGeneratorH
