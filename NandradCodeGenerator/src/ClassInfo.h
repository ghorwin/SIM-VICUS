#ifndef CLASSINFO_H
#define CLASSINFO_H

#include <vector>
#include <string>

#include <IBK_Path.h>

/*! Contains information parsed from a header file. */
class ClassInfo {
public:
	// Note: the Keyword-struct contains redundant data, for example the prefix in the category, which is
	//       the same for the entire class. This is mainly a convenience issue, so that all keywords from all
	//       files can be written together.
	struct Keyword {
		std::string		category;			// The enumeration type (full C++ type including class prefix)
		std::string		keyword;			// The keyword as string
		std::string		description;		// The english description text (optional)
		std::string		unit;				// The default unit (if any, otherwise an empty string)
		double			defaultValue =  std::numeric_limits<double>::quiet_NaN();		// The default value in default unit if any
		int				index;				// The index/enumeration number
		std::string		color;				// Color assigned to this keyword; default color is black #FFFFFF
	};

	struct EnumInfo {
		std::string		categoryName;		// The category name '<class>::<enum-name>'
		std::string		enumNUM;
		unsigned int	count;

		std::string enumType() const {
			std::string::size_type pos = categoryName.find("::");
			return categoryName.substr(pos+2);
		}
	};

	struct XMLInfo {
		bool			element;			// if false, an attribute is written
		bool			notEmpty = false;	// if true, string arguments are checked for emptiness after reading
		bool			required = false;	// if true, attribute/element must be present
		std::string		typeStr;			// C++ type string
		std::string		varName;			// Name of the variable without m_ prefix
	};


	/*! This function parses the given header file and extracts all data needed by the code generator. */
	bool parse(const IBK::Path & headerFilePath);

	IBK::Path				m_sourceHeaderFile;
	std::string				m_className;

	/*! Set to true if NANDRAD_READWRITE_PRIVATE macro is found in file. */
	bool					m_requirePrivateReadWrite = false;
	/*! Set to true, if NANDRAD_COMP macro is found in file. */
	bool					m_requireComparisonFunction = false;

	/*! Keywords collected from this class file. */
	std::vector<Keyword>	m_keywords;
	std::vector<EnumInfo>	m_enumInfo;
	std::vector<XMLInfo>	m_xmlInfo;

private:
	/*! Takes the string kwpara, searches for given delimiters (2) at end of string, and if presents takes
		the string between the delimiters and returns the string in as token. Also trimmes the kwpara string to exclude
		the extracted string and delimiters.
		If delimiters are not found, kwpara remains unmodified.
	*/
	void extractToken(std::string & kwpara, char delimiter, std::string & token);
};


#endif // CLASSINFO_H
